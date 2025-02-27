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

#include "lanelet2_map_validator/validators/crosswalk/check_pedestrian_traffic_light_subtype.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/BasicRegulatoryElements.h>

#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<PedestrianTrafficLightSubtypeValidator> reg;
}

lanelet::validation::Issues PedestrianTrafficLightSubtypeValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_pedestrian_traffic_light_subtype(map));

  return issues;
}

lanelet::validation::Issues
PedestrianTrafficLightSubtypeValidator::check_pedestrian_traffic_light_subtype(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & lane : map.laneletLayer) {
    if (lane.attributeOr(lanelet::AttributeName::Subtype, "") != std::string("crosswalk")) {
      continue;
    }

    for (const auto & reg_elem : lane.regulatoryElementsAs<lanelet::TrafficLight>()) {
      for (const auto & refer :
           reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers)) {
        if (refer.attributeOr(lanelet::AttributeName::Subtype, "") != std::string("red_green")) {
          issues.emplace_back(
            lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
            refer.id(),
            append_issue_code_prefix(
              this->name(), 1,
              "Subtype of pedestrian traffic lights should be set as \"red_green\"."));
        }
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
