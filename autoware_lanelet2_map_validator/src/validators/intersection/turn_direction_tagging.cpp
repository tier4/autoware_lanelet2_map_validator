// Copyright 2025 TIER IV, Inc.
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

#include "lanelet2_map_validator/validators/intersection/turn_direction_tagging.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry/geometry.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_core/primitives/BoundingBox.h>
#include <lanelet2_core/primitives/Polygon.h>

#include <map>
#include <set>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<IntersectionTurnDirectionTaggingValidator> reg;
}

lanelet::validation::Issues IntersectionTurnDirectionTaggingValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, checkTurnDirectionTagging(map));

  return issues;
}

lanelet::validation::Issues IntersectionTurnDirectionTaggingValidator::checkTurnDirectionTagging(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;
  const std::set<std::string> direction_set = {"left", "straight", "right"};

  for (const lanelet::ConstPolygon3d & polygon3d : map.polygonLayer) {
    if (
      !polygon3d.hasAttribute(lanelet::AttributeName::Type) ||
      polygon3d.attribute(lanelet::AttributeName::Type).value() != "intersection_area") {
      continue;
    }

    lanelet::BasicPolygon2d intersection_area2d = lanelet::traits::toBasicPolygon2d(polygon3d);
    lanelet::BoundingBox2d bbox2d = lanelet::geometry::boundingBox2d(intersection_area2d);
    lanelet::ConstLanelets nearby_lanelets = map.laneletLayer.search(bbox2d);

    for (const auto & lane : nearby_lanelets) {
      // Skip lanelets that are not roads and not inside the intersection_area
      if (
        !lane.hasAttribute(lanelet::AttributeName::Subtype) ||
        lane.attribute(lanelet::AttributeName::Subtype).value() !=
          lanelet::AttributeValueString::Road) {
        continue;
      }
      if (!lanelet_is_within_intersection_area2d(intersection_area2d, lane)) {
        continue;
      }

      if (!lane.hasAttribute("turn_direction")) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), lane.id()));
        continue;
      }

      std::string turn_direction = lane.attribute("turn_direction").value();
      if (direction_set.find(turn_direction) == direction_set.end()) {
        std::map<std::string, std::string> invalid_tag_map;
        invalid_tag_map["invalid_tag"] = turn_direction;
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 2), lane.id(), invalid_tag_map));
      }
    }
  }

  return issues;
}

bool IntersectionTurnDirectionTaggingValidator::lanelet_is_within_intersection_area2d(
  const lanelet::BasicPolygon2d & intersection_area2d, const lanelet::ConstLanelet & lanelet)
{
  for (const auto & left_point : lanelet.leftBound2d()) {
    if (!boost::geometry::covered_by(left_point.basicPoint(), intersection_area2d)) {
      return false;
    }
  }
  for (const auto & right_point : lanelet.rightBound2d()) {
    if (!boost::geometry::covered_by(right_point.basicPoint(), intersection_area2d)) {
      return false;
    }
  }
  return true;
}

}  // namespace lanelet::autoware::validation
