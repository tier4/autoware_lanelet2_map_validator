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

#include "lanelet2_map_validator/validators/traffic_light/regulatory_element_details_for_traffic_lights.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_validation/Validation.h>

#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RegulatoryElementsDetailsForTrafficLightsValidator> reg;
}  // namespace

lanelet::validation::Issues RegulatoryElementsDetailsForTrafficLightsValidator::operator()(
  const lanelet::LaneletMap & map)
{
  // All issues found by all validators
  lanelet::validation::Issues issues;

  // Append issues found by each validator
  lanelet::autoware::validation::appendIssues(issues, checkRegulatoryElementOfTrafficLights(map));
  return issues;
}

bool RegulatoryElementsDetailsForTrafficLightsValidator::isPedestrianTrafficLight(
  const lanelet::ConstLineStrings3d & traffic_lights)
{
  for (const auto & tl : traffic_lights) {
    const auto & attrs = tl.attributes();
    const auto & it = attrs.find(lanelet::AttributeName::Subtype);
    if (it == attrs.end() || it->second != "red_green") {
      return false;
    }
  }
  return true;
}

bool RegulatoryElementsDetailsForTrafficLightsValidator::isOneByOne(
  const lanelet::ConstLineStrings3d & traffic_lights,
  const lanelet::ConstLineStrings3d & light_bulbs)
{
  std::map<lanelet::Id, bool> is_referred_by_light_bulb;
  for (const auto & traffic_light : traffic_lights) {
    is_referred_by_light_bulb[traffic_light.id()] = false;
  }

  for (const auto & light_bulb : light_bulbs) {
    const int counterpart_id = light_bulb.attributeOr("traffic_light_id", 0);
    if (
      counterpart_id <= 0 ||
      is_referred_by_light_bulb.find(counterpart_id) == is_referred_by_light_bulb.end()) {
      return false;
    }
    is_referred_by_light_bulb.at(counterpart_id) = true;
  }

  bool result = true;
  for (const auto & [_, value] : is_referred_by_light_bulb) {
    result &= value;
  }
  return result;
}

lanelet::validation::Issues
RegulatoryElementsDetailsForTrafficLightsValidator::checkRegulatoryElementOfTrafficLights(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & elem : map.regulatoryElementLayer) {
    // Check through only traffic_light subtype regulatory_elements
    if (
      elem->attribute(lanelet::AttributeName::Subtype).value() !=
      std::string(lanelet::AttributeValueString::TrafficLight)) {
      continue;
    }

    // Get line strings of traffic light referred by regulatory element
    auto refers = elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers);
    // Get stop line referred by regulatory element
    auto ref_lines = elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine);
    // Get light bulbs referred by regulatory element
    auto light_bulbs = elem->getParameters<lanelet::ConstLineString3d>("light_bulbs");

    // Report error if regulatory element does not have stop line and this is not a pedestrian
    // traffic light
    if (ref_lines.empty() && !isPedestrianTrafficLight(refers)) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), elem->id()));
    }

    for (const auto & refer : refers) {
      if (
        refer.attributeOr(lanelet::AttributeName::Type, "") !=
        std::string(lanelet::AttributeValueString::TrafficLight)) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), refer.id()));
      }
    }

    for (const auto & ref_line : ref_lines) {
      if (
        ref_line.attributeOr(lanelet::AttributeName::Type, "") !=
        std::string(lanelet::AttributeValueString::StopLine)) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 3), ref_line.id()));
      }
    }

    for (const auto & light_bulb : light_bulbs) {
      if (light_bulb.attributeOr(lanelet::AttributeName::Type, "") != std::string("light_bulbs")) {
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 4), light_bulb.id()));
      }

      if (!light_bulb.hasAttribute("traffic_light_id")) {
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 5), light_bulb.id()));
      }
    }

    if (refers.size() != light_bulbs.size()) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 6), elem->id()));
    } else if (!isOneByOne(refers, light_bulbs)) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 7), elem->id()));
    }
  }
  return issues;
}

}  // namespace lanelet::autoware::validation
