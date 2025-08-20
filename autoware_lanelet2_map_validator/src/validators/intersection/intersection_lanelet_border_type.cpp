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

#include "lanelet2_map_validator/validators/intersection/intersection_lanelet_border_type.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>

#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<IntersectionLaneletBorderTypeValidator> reg;
}

lanelet::validation::Issues IntersectionLaneletBorderTypeValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_intersection_lanelet_border_type(map));

  return issues;
}

lanelet::validation::Issues
IntersectionLaneletBorderTypeValidator::check_intersection_lanelet_border_type(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lanelet : map.laneletLayer) {
    if (lanelet.hasAttribute("intersection_area")) {
      const std::string left_type =
        lanelet.leftBound().attributeOr(lanelet::AttributeName::Type, "");
      if (left_type != "virtual" && left_type != "road_border") {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), lanelet.id()));
        continue;  // skip right bound check if left bound is already invalid
      }

      const std::string right_type =
        lanelet.rightBound().attributeOr(lanelet::AttributeName::Type, "");
      if (right_type != "virtual" && right_type != "road_border") {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), lanelet.id()));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
