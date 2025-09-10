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

#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/intersects.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_core/primitives/Polygon.h>

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

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

  // Forward check: For each intersection area, check covered lanelets have correct tags
  for (const lanelet::ConstPolygon3d & polygon3d : map.polygonLayer) {
    if (polygon3d.attributeOr(lanelet::AttributeName::Type, "none") != "intersection_area") {
      continue;
    }

    lanelet::BasicPolygon2d area_polygon2d = lanelet::traits::to2D(polygon3d.basicPolygon());

    // Get bounding box for spatial filtering
    boost::geometry::model::box<lanelet::BasicPoint2d> area_bbox;
    boost::geometry::envelope(area_polygon2d, area_bbox);

    // Filter lanelets by bounding box intersection first
    std::vector<lanelet::ConstLanelet> candidate_lanelets;
    for (const lanelet::ConstLanelet & lanelet : map.laneletLayer) {
      lanelet::BasicPolygon2d lanelet_polygon = lanelet.polygon2d().basicPolygon();
      boost::geometry::model::box<lanelet::BasicPoint2d> lanelet_bbox;
      boost::geometry::envelope(lanelet_polygon, lanelet_bbox);

      if (boost::geometry::intersects(area_bbox, lanelet_bbox)) {
        candidate_lanelets.push_back(lanelet);
      }
    }

    // Check precise coverage for candidate lanelets
    for (const lanelet::ConstLanelet & lanelet : candidate_lanelets) {
      lanelet::BasicPolygon2d lanelet_polygon = lanelet.polygon2d().basicPolygon();
      if (boost::geometry::covered_by(lanelet_polygon, area_polygon2d)) {
        std::string intersection_area_attr = lanelet.attributeOr("intersection_area", "none");
        if (intersection_area_attr == "none") {
          // Issue-001: Lanelet missing intersection_area tag
          std::map<std::string, std::string> area_id_map;
          area_id_map["area_id"] = std::to_string(polygon3d.id());
          issues.emplace_back(
            construct_issue_from_code(issue_code(this->name(), 1), lanelet.id(), area_id_map));
        } else {
          lanelet::Id tagged_area_id =
            static_cast<lanelet::Id>(std::atoi(intersection_area_attr.c_str()));
          if (tagged_area_id != polygon3d.id()) {
            // Issue-002: Lanelet has wrong intersection_area tag
            std::map<std::string, std::string> tag_map;
            tag_map["expected_area_id"] = std::to_string(polygon3d.id());
            tag_map["actual_area_id"] = intersection_area_attr;
            issues.emplace_back(
              construct_issue_from_code(issue_code(this->name(), 2), lanelet.id(), tag_map));
          }
        }
      }
    }
  }

  // Issue-003: Lanelet has intersection_area tag but is not completely covered by the referenced
  // area
  for (const lanelet::ConstLanelet & lanelet : map.laneletLayer) {
    lanelet::id tagged_area_id = lanelet.attributeOr("intersection_area", lanelet::InvalId);
    if (tagged_area_id != lanelet::InvalId) {
      auto area_it = intersection_areas.find(tagged_area_id);
      if (area_it == intersection_areas.end()) {
        continue;
      }

      lanelet::BasicPolygon2d area_polygon2d =
        lanelet::traits::to2D(area_it->second.basicPolygon());
      lanelet::BasicPolygon2d lanelet_polygon = lanelet.polygon2d().basicPolygon();
      if (!boost::geometry::covered_by(lanelet_polygon, area_polygon2d)) {
        std::map<std::string, std::string> area_id_map;
        area_id_map["area_id"] = std::to_string(tagged_area_id);
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 3), lanelet.id(), area_id_map));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
