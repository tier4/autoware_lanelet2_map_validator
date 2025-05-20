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

#include "lanelet2_map_validator/validators/intersection/single_virtual_traffic_light_reference.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

#include <lanelet2_core/LaneletMap.h>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<SingleVirtualTrafficLightReferenceValidator> reg;
}

lanelet::validation::Issues SingleVirtualTrafficLightReferenceValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(
    issues, check_single_virtual_traffic_light_reference(map));

  return issues;
}

lanelet::validation::Issues
SingleVirtualTrafficLightReferenceValidator::check_single_virtual_traffic_light_reference(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lane : map.laneletLayer) {
    const auto reg_elems = lane.regulatoryElementsAs<VirtualTrafficLight>();
    if (reg_elems.empty()) {
      continue;
    }

    if (reg_elems.size() > 1) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), lane.id()));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
