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

#include "lanelet2_map_validator/validators/traffic_light/light_bulb_tagging.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>

#include <set>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<LightBulbsTaggingValidator> reg;
}

lanelet::validation::Issues LightBulbsTaggingValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_light_bulb_tagging(map));

  return issues;
}

lanelet::validation::Issues LightBulbsTaggingValidator::check_light_bulb_tagging(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  std::set<lanelet::Id> validated_bulbs;

  // Search from the regulatory element layer since the linestring layer might be too huge
  for (const auto & reg_elem : map.regulatoryElementLayer) {
    const auto light_bulbs_linestrings =
      reg_elem->getParameters<lanelet::ConstLineString3d>("light_bulbs");
    for (const lanelet::ConstLineString3d & light_bulbs : light_bulbs_linestrings) {
      if (validated_bulbs.find(light_bulbs.id()) != validated_bulbs.end()) {
        continue;
      }
      validated_bulbs.insert(light_bulbs.id());

      for (const lanelet::ConstPoint3d & bulb : light_bulbs) {
        if (!bulb.hasAttribute("color")) {
          issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), bulb.id()));
        } else if (
          expected_colors_.find(bulb.attribute("color").value()) == expected_colors_.end()) {
          issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), bulb.id()));
        }

        if (
          bulb.hasAttribute("arrow") &&
          expected_arrows_.find(bulb.attribute("arrow").value()) == expected_arrows_.end()) {
          issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 3), bulb.id()));
        }
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
