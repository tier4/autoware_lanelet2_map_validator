// Copyright 2024 Autoware Foundation
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

#include "lanelet2_map_validator/validators/intersection/right_of_way_with_traffic_lights.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/RegulatoryElement.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RightOfWayWithTrafficLightsValidator> reg;
}

lanelet::validation::Issues RightOfWayWithTrafficLightsValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_right_of_way_with_traffic_lights(map));

  return issues;
}

lanelet::validation::Issues RightOfWayWithTrafficLightsValidator::check_right_of_way_with_traffic_lights(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const lanelet::ConstLanelet & lanelet : map.laneletLayer) {
    if (!lanelet.hasAttribute("turn_direction")) {
      continue;
    }
    
    auto turn_direction = lanelet.attribute("turn_direction").value();
    if (turn_direction != "left" && turn_direction != "right") {
      continue;
    }

    bool has_traffic_light_ref = false;
    for (const auto & reg_elem : lanelet.regulatoryElements()) {
      if (reg_elem->hasAttribute("subtype") && reg_elem->attribute("subtype").value() == "traffic_light") {
        has_traffic_light_ref = true;
        break;
      }
    }

    if (!has_traffic_light_ref) {
      continue;
    }

    bool has_right_of_way_ref = false;
    lanelet::RegulatoryElementConstPtr right_of_way_elem = nullptr;

    for (const auto & reg_elem : lanelet.regulatoryElements()) {
      if (reg_elem->hasAttribute("subtype") && reg_elem->attribute("subtype").value() == "right_of_way") {
        has_right_of_way_ref = true;
        right_of_way_elem = reg_elem;
        break;
      }
    }

    if (!has_right_of_way_ref) {
      // Issue-001: Lanelet with turn_direction and traffic_light reference missing right_of_way reference
      std::map<std::string, std::string> reason_map;
      reason_map["turn_direction"] = lanelet.attribute("turn_direction").value();
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), lanelet.id(), reason_map));
      continue;
    }

    bool is_set_as_right_of_way = false;
    auto right_of_way_lanelets = right_of_way_elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::RightOfWay);
    for (const auto & param : right_of_way_lanelets) {
      if (param.id() == lanelet.id()) {
        is_set_as_right_of_way = true;
        break;
      }
    }

    if (!is_set_as_right_of_way) {
      // issue-002: right_of_way regulatory element doesn't set this lanelet as right_of_way role
      std::map<std::string, std::string> reason_map;
      reason_map["turn_direction"] = lanelet.attribute("turn_direction").value();
      reason_map["right_of_way_id"] = std::to_string(right_of_way_elem->id());
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 2), lanelet.id(), reason_map));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
