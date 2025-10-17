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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__AREA__MISSING_REGULATORY_ELEMENTS_FOR_BUS_STOP_AREAS_HPP_  // NOLINT
#define LANELET2_MAP_VALIDATOR__VALIDATORS__AREA__MISSING_REGULATORY_ELEMENTS_FOR_BUS_STOP_AREAS_HPP_  // NOLINT

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

namespace lanelet::autoware::validation
{
class MissingRegulatoryElementsForBusStopAreas : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name()
  {
    return "mapping.area.missing_regulatory_elements_for_bus_stop_areas";
  }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
  lanelet::validation::Issues check_missing_regulatory_elements_for_bus_stop_areas(
    const lanelet::LaneletMap & map);
};
}  // namespace lanelet::autoware::validation

// clang-format off
#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__AREA__MISSING_REGULATORY_ELEMENTS_FOR_BUS_STOP_AREAS_HPP_  // NOLINT
// clang-format on
