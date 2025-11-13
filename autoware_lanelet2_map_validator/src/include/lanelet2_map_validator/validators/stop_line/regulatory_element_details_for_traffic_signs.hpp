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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__STOP_LINE__REGULATORY_ELEMENT_DETAILS_FOR_TRAFFIC_SIGNS_HPP_  // NOLINT
#define LANELET2_MAP_VALIDATOR__VALIDATORS__STOP_LINE__REGULATORY_ELEMENT_DETAILS_FOR_TRAFFIC_SIGNS_HPP_  // NOLINT

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

namespace lanelet::autoware::validation
{
class RegulatoryElementDetailsForTrafficSignsValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name()
  {
    return "mapping.stop_line.regulatory_element_details_for_traffic_signs";
  }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  RegulatoryElementDetailsForTrafficSignsValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    max_bounding_box_size_ = get_parameter_or<double>(parameters, "max_bounding_box_size", 40.0);
  }

private:
  lanelet::validation::Issues check_regulatory_element_details_for_traffic_signs(
    const lanelet::LaneletMap & map);
  double max_bounding_box_size_;
};
}  // namespace lanelet::autoware::validation

// clang-format off
#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__STOP_LINE__REGULATORY_ELEMENT_DETAILS_FOR_TRAFFIC_SIGNS_HPP_  // NOLINT
// clang-format on
