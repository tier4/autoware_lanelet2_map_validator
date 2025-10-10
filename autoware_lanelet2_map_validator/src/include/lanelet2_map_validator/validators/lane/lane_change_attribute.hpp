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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANE_CHANGE_ATTRIBUTE_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANE_CHANGE_ATTRIBUTE_HPP_

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

namespace lanelet::autoware::validation
{
class LaneChangeAttributeValidator : public lanelet::validation::MapValidator
{
public:
  constexpr static const char * name() { return "mapping.lane.lane_change_attribute"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
  lanelet::validation::Issues check_lane_change_attribute(const lanelet::LaneletMap & map);
};
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANE_CHANGE_ATTRIBUTE_HPP_
