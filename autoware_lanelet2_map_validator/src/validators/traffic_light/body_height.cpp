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

#include "lanelet2_map_validator/validators/traffic_light/body_height.hpp"

#include "lanelet2_map_validator/config_store.hpp"
#include "lanelet2_map_validator/utils.hpp"

#include <map>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<BodyHeightValidator> reg;
}

lanelet::validation::Issues BodyHeightValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_body_height(map));

  return issues;
}

lanelet::validation::Issues BodyHeightValidator::check_body_height(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & linestring : map.lineStringLayer) {
    if (
      !linestring.hasAttribute(lanelet::AttributeName::Type) ||
      linestring.attribute(lanelet::AttributeName::Type).value() !=
        lanelet::AttributeValueString::TrafficLight) {
      continue;
    }

    if (!linestring.hasAttribute("height")) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), linestring.id()));
      continue;
    }

    const double height_value = linestring.attributeOr("height", -1.0);

    if (height_value < min_height_ || height_value > max_height_) {
      std::map<std::string, std::string> reason_map;
      reason_map["min_height"] = std::to_string(min_height_);
      reason_map["max_height"] = std::to_string(max_height_);
      reason_map["actual_height"] = std::to_string(height_value);
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 2), linestring.id(), reason_map));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
