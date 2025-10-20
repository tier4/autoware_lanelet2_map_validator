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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__BODY_HEIGHT_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__BODY_HEIGHT_HPP_

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

namespace lanelet::autoware::validation
{
class BodyHeightValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name() { return "mapping.traffic_light.body_height"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  BodyHeightValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    min_height_ = get_parameter_or<double>(parameters, "min_height", 0.2);
    max_height_ = get_parameter_or<double>(parameters, "max_height", 2.0);
  }

private:
  lanelet::validation::Issues check_body_height(const lanelet::LaneletMap & map);

  double min_height_;
  double max_height_;
};
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__BODY_HEIGHT_HPP_
