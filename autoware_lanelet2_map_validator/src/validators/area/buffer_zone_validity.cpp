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

#include "lanelet2_map_validator/validators/area/buffer_zone_validity.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/primitives/Polygon.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<BufferZoneValidity> reg;
}

lanelet::validation::Issues BufferZoneValidity::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_buffer_zone_validity(map));

  return issues;
}

lanelet::validation::Issues BufferZoneValidity::check_buffer_zone_validity(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & polygon : map.polygonLayer) {
    if (
      !polygon.hasAttribute(lanelet::AttributeName::Type) ||
      polygon.attribute(lanelet::AttributeName::Type).value() != "hatched_road_markings") {
      continue;
    }

    lanelet::BoundingBox2d bbox2d =
      lanelet::geometry::boundingBox2d(lanelet::traits::to2D(polygon.basicPolygon()));
    lanelet::ConstLanelets nearby_lanelets = map.laneletLayer.search(bbox2d);

    std::set<lanelet::Id> lanelet_point_ids;
    lanelet::BasicPolygon2d buffer_poly2d = lanelet::traits::to2D(polygon.basicPolygon());
    double buffer_area = boost::geometry::area(buffer_poly2d);

    for (const auto & ll : nearby_lanelets) {
      // check for point sharing (Issue-001)
      for (const auto & lineString : {ll.leftBound(), ll.rightBound()}) {
        for (const auto & pt : lineString) {
          lanelet_point_ids.insert(pt.id());
        }
      }

      // check for road/road_shoulder overlap (Issue-004)
      std::string subtype = ll.attributeOr(lanelet::AttributeName::Subtype, "");
      if (subtype == "road" || subtype == "road_shoulder") {
        lanelet::BasicPolygon2d lanelet_polygon = ll.polygon2d().basicPolygon();

        if (boost::geometry::intersects(buffer_poly2d, lanelet_polygon)) {
          std::vector<lanelet::BasicPolygon2d> intersection_result;
          boost::geometry::intersection(buffer_poly2d, lanelet_polygon, intersection_result);

          if (!intersection_result.empty()) {
            double intersection_area = 0.0;
            for (const auto & poly : intersection_result) {
              intersection_area += boost::geometry::area(poly);
            }

            double overlap_threshold = 0.01;
            double overlap_ratio = intersection_area / buffer_area;

            if (overlap_ratio > overlap_threshold) {
              std::map<std::string, std::string> overlap_map;
              overlap_map["lanelet_id"] = std::to_string(ll.id());
              overlap_map["lanelet_subtype"] = subtype;
              issues.emplace_back(
                construct_issue_from_code(issue_code(this->name(), 4), polygon.id(), overlap_map));
            }
          }
        }
      }
    }

    // Issue-001
    std::vector<lanelet::Id> unshared_point_ids;
    for (const auto & pt : polygon) {
      if (lanelet_point_ids.find(pt.id()) == lanelet_point_ids.end()) {
        unshared_point_ids.push_back(pt.id());
      }
    }

    if (!unshared_point_ids.empty()) {
      std::map<std::string, std::string> point_ids_map;
      std::ostringstream ids_stream;

      for (size_t i = 0; i < unshared_point_ids.size(); ++i) {
        ids_stream << unshared_point_ids[i];
        if (i < unshared_point_ids.size() - 1) {
          ids_stream << ", ";
        }
      }

      point_ids_map["point_ids"] = ids_stream.str();
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), polygon.id(), point_ids_map));
    }

    lanelet::ConstPolygons3d nearby_polygons = map.polygonLayer.search(bbox2d);
    for (const lanelet::ConstPolygon3d & intersection_poly : nearby_polygons) {
      if (
        !intersection_poly.hasAttribute(lanelet::AttributeName::Type) ||
        intersection_poly.attribute(lanelet::AttributeName::Type).value() != "intersection_area") {
        continue;
      }
      lanelet::BasicPolygon2d intersection_poly2d =
        lanelet::traits::to2D(intersection_poly.basicPolygon());

      // Issue-002
      if (boost::geometry::intersects(buffer_poly2d, intersection_poly2d)) {
        if (
          !boost::geometry::covered_by(buffer_poly2d, intersection_poly2d) &&
          !boost::geometry::covered_by(intersection_poly2d, buffer_poly2d)) {
          std::map<std::string, std::string> overlap_map;
          overlap_map["intersection_area_id"] = std::to_string(intersection_poly.id());
          issues.emplace_back(
            construct_issue_from_code(issue_code(this->name(), 2), polygon.id(), overlap_map));
        }
      }
    }

    // Issue-003
    boost::geometry::validity_failure_type failure_type;
    bool polygon_is_valid = boost::geometry::is_valid(buffer_poly2d, failure_type);
    if (
      !polygon_is_valid &&
      failure_type != boost::geometry::validity_failure_type::failure_wrong_orientation) {
      std::map<std::string, std::string> reason_map;
      reason_map["boost_geometry_message"] =
        boost::geometry::validity_failure_type_message(failure_type);
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 3), polygon.id(), reason_map));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
