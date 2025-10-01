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

#include "lanelet2_map_validator/validators/lane/road_lanelet_attribute.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RoadLaneletAttributeValidator> reg;
}

lanelet::validation::Issues RoadLaneletAttributeValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_road_lanelet_attribute(map));

  return issues;
}

lanelet::validation::Issues RoadLaneletAttributeValidator::check_road_lanelet_attribute(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lanelet : map.laneletLayer) {
    if (
      !lanelet.hasAttribute(lanelet::AttributeName::Subtype) ||
      lanelet.attribute(lanelet::AttributeName::Subtype).value() !=
        lanelet::AttributeValueString::Road) {
      continue;
    }

    if (
      !lanelet.hasAttribute("location") || (lanelet.attribute("location").value() != "urban" &&
                                            lanelet.attribute("location").value() != "private")) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), lanelet.id()));
    }

    if (!lanelet.hasAttribute("one_way") || lanelet.attribute("one_way").value() != "yes") {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), lanelet.id()));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
