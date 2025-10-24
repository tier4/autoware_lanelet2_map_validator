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

#include "lanelet2_map_validator/validators/lane/speed_limit_validity.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <map>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<SpeedLimitValidityValidator> reg;
}

lanelet::validation::Issues SpeedLimitValidityValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_speed_limit_validity(map));

  return issues;
}

lanelet::validation::Issues SpeedLimitValidityValidator::check_speed_limit_validity(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lanelet : map.laneletLayer) {
    if (
      !lanelet.hasAttribute(lanelet::AttributeName::Subtype) ||
      (lanelet.attribute(lanelet::AttributeName::Subtype).value() !=
         lanelet::AttributeValueString::Road &&
       lanelet.attribute(lanelet::AttributeName::Subtype).value() !=
         lanelet::AttributeValueString::Private)) {
      continue;
    }

    const std::string subtype = lanelet.attribute(lanelet::AttributeName::Subtype).value();

    if (lanelet.hasAttribute("speed_limit")) {
      const std::string speed_limit_str = lanelet.attribute("speed_limit").value();

      try {
        double speed_limit = std::stod(speed_limit_str);
        if (speed_limit <= 0.0) {
          // Issue-001: speed_limit is not positive
          std::map<std::string, std::string> substitution_map;
          substitution_map["speed_limit_value"] = speed_limit_str;
          substitution_map["subtype"] = subtype;
          issues.emplace_back(
            construct_issue_from_code(issue_code(this->name(), 1), lanelet.id(), substitution_map));
        }
      } catch (const std::exception &) {
        // Issue-001: speed_limit is not a valid number
        std::map<std::string, std::string> substitution_map;
        substitution_map["speed_limit_value"] = speed_limit_str;
        substitution_map["subtype"] = subtype;
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 1), lanelet.id(), substitution_map));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
