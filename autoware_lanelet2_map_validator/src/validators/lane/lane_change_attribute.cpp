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

#include "lanelet2_map_validator/validators/lane/lane_change_attribute.hpp"

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
lanelet::validation::RegisterMapValidator<LaneChangeAttributeValidator> reg;
}

lanelet::validation::Issues LaneChangeAttributeValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_lane_change_attribute(map));

  return issues;
}

lanelet::validation::Issues LaneChangeAttributeValidator::check_lane_change_attribute(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  std::unordered_set<Id> checked_bounds;

  auto check_bound = [&](
                       const lanelet::ConstLineString3d & bound, const std::string & bound_type,
                       const lanelet::ConstLanelet & current_lanelet) {
    if (checked_bounds.find(bound.id()) != checked_bounds.end()) {
      return;
    }

    checked_bounds.insert(bound.id());

    lanelet::BoundingBox2d bbox = lanelet::geometry::boundingBox2d(bound);
    lanelet::ConstLanelets nearby_lanelets = map.laneletLayer.search(bbox);

    bool is_shared = false;
    for (const auto & nearby : nearby_lanelets) {
      if (
        nearby.id() == current_lanelet.id() ||
        !nearby.hasAttribute(lanelet::AttributeName::Subtype) ||
        nearby.attribute(lanelet::AttributeName::Subtype).value() !=
          lanelet::AttributeValueString::Road) {
        continue;
      }

      if (bound.id() == nearby.leftBound().id() || bound.id() == nearby.rightBound().id()) {
        is_shared = true;
        break;
      }
    }

    if (!is_shared) {
      return;
    }

    if (!bound.hasAttribute("lane_change")) {
      std::map<std::string, std::string> bound_type_map;
      bound_type_map["bound_type"] = bound_type;
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), bound.id(), bound_type_map));
    } else {
      const std::string lane_change_value = bound.attribute("lane_change").value();
      if (lane_change_value != "yes" && lane_change_value != "no") {
        std::map<std::string, std::string> substitution_map;
        substitution_map["bound_type"] = bound_type;
        substitution_map["invalid_value"] = lane_change_value;
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 2), bound.id(), substitution_map));
      }
    }
  };

  for (const auto & lanelet : map.laneletLayer) {
    if (
      !lanelet.hasAttribute(lanelet::AttributeName::Subtype) ||
      lanelet.attribute(lanelet::AttributeName::Subtype).value() !=
        lanelet::AttributeValueString::Road) {
      continue;
    }

    check_bound(lanelet.leftBound(), "left", lanelet);
    check_bound(lanelet.rightBound(), "right", lanelet);
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
