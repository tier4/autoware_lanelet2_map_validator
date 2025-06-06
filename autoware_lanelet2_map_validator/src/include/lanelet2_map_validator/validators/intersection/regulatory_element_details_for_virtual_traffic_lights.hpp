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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__REGULATORY_ELEMENT_DETAILS_FOR_VIRTUAL_TRAFFIC_LIGHTS_HPP_  // NOLINT
#define LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__REGULATORY_ELEMENT_DETAILS_FOR_VIRTUAL_TRAFFIC_LIGHTS_HPP_  // NOLINT

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
class RegulatoryElementDetailsForVirtualTrafficLightsValidator
: public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name()
  {
    return "mapping.intersection.regulatory_element_details_for_virtual_traffic_lights";
  }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  RegulatoryElementDetailsForVirtualTrafficLightsValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    available_refers_type_ = get_parameter_or<std::vector<std::string>>(
      parameters, "available_refers_type", {"intersection_coordination"});
  }

private:
  lanelet::validation::Issues check_regulatory_element_details_for_virtual_traffic_lights(
    const lanelet::LaneletMap & map);
  std::vector<std::string> available_refers_type_;
};
}  // namespace lanelet::autoware::validation

// clang-format off
#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__REGULATORY_ELEMENT_DETAILS_FOR_VIRTUAL_TRAFFIC_LIGHTS_HPP_  // NOLINT
// clang-format on
