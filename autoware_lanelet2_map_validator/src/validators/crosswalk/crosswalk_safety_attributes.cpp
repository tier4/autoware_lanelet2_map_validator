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

#include "lanelet2_map_validator/validators/crosswalk/crosswalk_safety_attributes.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <map>
#include <stdexcept>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<CrosswalkSafetyAttributesValidator> reg;
}

lanelet::validation::Issues CrosswalkSafetyAttributesValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_crosswalk_safety_attributes(map));

  return issues;
}

lanelet::validation::Issues CrosswalkSafetyAttributesValidator::check_crosswalk_safety_attributes(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & ll : map.laneletLayer) {
    const auto & attrs = ll.attributes();
    const auto & subtype_it = attrs.find(lanelet::AttributeName::Subtype);

    if (
      subtype_it == attrs.end() || subtype_it->second != lanelet::AttributeValueString::Crosswalk) {
      continue;
    }

    // Issue-001: safety_slow_down_speed attribute
    const auto & speed_it = attrs.find("safety_slow_down_speed");
    if (speed_it != attrs.end()) {
      const std::string & speed_value = speed_it->second.value();
      try {
        size_t idx = 0;
        double speed = std::stod(speed_value, &idx);
        if (idx != speed_value.length() || speed <= 0.0) {
          std::map<std::string, std::string> reason_map;
          reason_map["attribute_value"] = speed_value;
          issues.emplace_back(
            construct_issue_from_code(issue_code(this->name(), 1), ll.id(), reason_map));
        }
      } catch (const std::exception &) {
        std::map<std::string, std::string> reason_map;
        reason_map["attribute_value"] = speed_value;
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 1), ll.id(), reason_map));
      }
    }

    // Issue-002: safety_slow_down_distance attribute
    const auto & distance_it = attrs.find("safety_slow_down_distance");
    if (distance_it != attrs.end()) {
      const std::string & distance_value = distance_it->second.value();
      try {
        size_t idx = 0;
        double distance = std::stod(distance_value, &idx);
        if (idx != distance_value.length() || distance <= 0.0) {
          std::map<std::string, std::string> reason_map;
          reason_map["attribute_value"] = distance_value;
          issues.emplace_back(
            construct_issue_from_code(issue_code(this->name(), 2), ll.id(), reason_map));
        }
      } catch (const std::exception &) {
        std::map<std::string, std::string> reason_map;
        reason_map["attribute_value"] = distance_value;
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 2), ll.id(), reason_map));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
