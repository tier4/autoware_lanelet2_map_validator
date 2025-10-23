// Copyright 2024-2025 TIER IV, Inc.
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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__REGULATORY_ELEMENT_DETAILS_FOR_TRAFFIC_LIGHTS_HPP_  // NOLINT
#define LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__REGULATORY_ELEMENT_DETAILS_FOR_TRAFFIC_LIGHTS_HPP_  // NOLINT

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <vector>

namespace lanelet::autoware::validation
{
class RegulatoryElementsDetailsForTrafficLightsValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name()
  {
    return "mapping.traffic_light.regulatory_element_details";
  }

  RegulatoryElementsDetailsForTrafficLightsValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    max_bounding_box_size_ = get_parameter_or<double>(parameters, "max_bounding_box_size", 40.0);
  }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
  bool isPedestrianTrafficLight(const lanelet::ConstLineStrings3d & traffic_lights);

  /**
   * @brief return true if all of the traffic_lights are referred by light_bulbs through
   * "traffic_light_id" attribute
   */
  bool isOneByOne(
    const lanelet::ConstLineStrings3d & traffic_lights,
    const lanelet::ConstLineStrings3d & light_bulbs);
  lanelet::validation::Issues checkRegulatoryElementOfTrafficLights(
    const lanelet::LaneletMap & map);

  double max_bounding_box_size_;
};
}  // namespace lanelet::autoware::validation

// clang-format off
#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__REGULATORY_ELEMENT_DETAILS_FOR_TRAFFIC_LIGHTS_HPP_  // NOLINT
// clang-format on
