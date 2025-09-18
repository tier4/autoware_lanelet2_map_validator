// Copyright 2025 Autoware Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lanelet2_map_validator/validators/intersection/intersection_area_tagging.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/intersects.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/BoundingBox.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_core/primitives/Polygon.h>

#include <map>
#include <string>
#include <unordered_set>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<IntersectionAreaTaggingValidator> reg;
}

lanelet::validation::Issues IntersectionAreaTaggingValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_intersection_area_tagging(map));

  return issues;
}

lanelet::validation::Issues IntersectionAreaTaggingValidator::check_intersection_area_tagging(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const lanelet::ConstPolygon3d & polygon3d : map.polygonLayer) {
    if (
      polygon3d.attributeOr(lanelet::AttributeName::Type, "none") !=
      std::string("intersection_area")) {
      continue;
    }

    lanelet::BasicPolygon2d area_polygon2d = lanelet::traits::toBasicPolygon2d(polygon3d);
    lanelet::BoundingBox2d bbox2d = lanelet::geometry::boundingBox2d(area_polygon2d);
    lanelet::ConstLanelets nearby_lanelets = map.laneletLayer.search(bbox2d);

    for (const lanelet::ConstLanelet & lanelet : nearby_lanelets) {
      lanelet::BasicPolygon2d lanelet_polygon = lanelet.polygon2d().basicPolygon();
      boost::geometry::correct(lanelet_polygon);
      boost::geometry::correct(area_polygon2d);

      if (boost::geometry::intersects(lanelet_polygon, area_polygon2d)) {
        std::vector<lanelet::BasicPolygon2d> intersection_result;
        boost::geometry::intersection(lanelet_polygon, area_polygon2d, intersection_result);
        if (intersection_result.empty()) {
          continue;
        } else {
          double lanelet_area = boost::geometry::area(lanelet_polygon);
          double intersection_area = 0.0;
          for (const auto & poly : intersection_result) {
            intersection_area += boost::geometry::area(poly);
          }

          double coverage_threshold = 0.99;
          double lanelet_coverage = intersection_area / lanelet_area;

          if (lanelet_coverage >= coverage_threshold) {
            lanelet::Id tagged_area_id = lanelet.attributeOr("intersection_area", lanelet::InvalId);
            if (tagged_area_id == lanelet::InvalId) {
              // Issue-001: Lanelet missing intersection_area tag
              std::map<std::string, std::string> area_id_map;
              area_id_map["area_id"] = std::to_string(polygon3d.id());
              issues.emplace_back(
                construct_issue_from_code(issue_code(this->name(), 1), lanelet.id(), area_id_map));
            } else {
              // Direct ID comparison
              if (tagged_area_id != polygon3d.id()) {
                // Issue-002: Lanelet has wrong intersection_area tag
                std::map<std::string, std::string> tag_map;
                tag_map["expected_area_id"] = std::to_string(polygon3d.id());
                tag_map["actual_area_id"] = std::to_string(tagged_area_id);
                issues.emplace_back(
                  construct_issue_from_code(issue_code(this->name(), 2), lanelet.id(), tag_map));
              }
            }
          }
        }
      }
    }
  }

  // Issue-003: Lanelet has intersection_area tag but is not completely covered by the referenced
  // area
  for (const lanelet::ConstLanelet & lanelet : map.laneletLayer) {
    lanelet::Id tagged_area_id = lanelet.attributeOr("intersection_area", lanelet::InvalId);
    if (tagged_area_id == lanelet::InvalId) {
      continue;
    }
    bool found_area = false;
    lanelet::BasicPolygon2d area_polygon2d;

    if (map.polygonLayer.exists(tagged_area_id)) {
      lanelet::ConstPolygon3d polygon3d = map.polygonLayer.get(tagged_area_id);
      if (
        polygon3d.attributeOr(lanelet::AttributeName::Type, "none") ==
        std::string("intersection_area")) {
        area_polygon2d = lanelet::traits::toBasicPolygon2d(polygon3d);
        found_area = true;
      }
    }

    if (!found_area) {
      continue;  // (should be caught by dangling reference validator)
    }

    lanelet::BasicPolygon2d lanelet_polygon = lanelet.polygon2d().basicPolygon();
    boost::geometry::correct(lanelet_polygon);
    boost::geometry::correct(area_polygon2d);

    if (boost::geometry::intersects(lanelet_polygon, area_polygon2d)) {
      std::vector<lanelet::BasicPolygon2d> intersection_result;
      boost::geometry::intersection(area_polygon2d, lanelet_polygon, intersection_result);
      if (intersection_result.empty()) {
        continue;
      } else {
        double lanelet_area = boost::geometry::area(lanelet_polygon);
        double intersection_area = 0.0;
        for (const auto & poly : intersection_result) {
          intersection_area += boost::geometry::area(poly);
        }

        double coverage_threshold = 0.99;
        double lanelet_coverage = intersection_area / lanelet_area;

        if (lanelet_coverage < coverage_threshold) {
          std::map<std::string, std::string> area_id_map;
          area_id_map["area_id"] = std::to_string(tagged_area_id);
          area_id_map["coverage_percentage"] = std::to_string(lanelet_coverage * 100.0) + "%";
          issues.emplace_back(
            construct_issue_from_code(issue_code(this->name(), 3), lanelet.id(), area_id_map));
        }
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
