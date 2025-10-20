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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__CENTERLINE_GEOMETRY_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__CENTERLINE_GEOMETRY_HPP_

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <string>

namespace lanelet::autoware::validation
{
class CenterlineGeometryValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name() { return "mapping.lane.centerline_geometry"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  CenterlineGeometryValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    const auto dimension_mode_str =
      get_parameter_or<std::string>(parameters, "dimension_mode", "2D");
    dimension_mode_ = (dimension_mode_str == "3D") ? threeD : twoD;
    planar_threshold_ = get_parameter_or<double>(parameters, "planar_threshold", 1e-6);
    height_threshold_ = get_parameter_or<double>(parameters, "height_threshold", 1e-6);
  }

private:
  lanelet::validation::Issues check_centerline_geometry(const lanelet::LaneletMap & map);

  enum Mode { twoD, threeD };

  Mode dimension_mode_;
  double planar_threshold_;
  double height_threshold_;
};
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__CENTERLINE_GEOMETRY_HPP_
