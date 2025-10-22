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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__LIGHT_BULB_TAGGING_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__LIGHT_BULB_TAGGING_HPP_

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <set>
#include <string>

namespace lanelet::autoware::validation
{
class LightBulbsTaggingValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name() { return "mapping.traffic_light.light_bulb_tagging"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
  lanelet::validation::Issues check_light_bulb_tagging(const lanelet::LaneletMap & map);

  inline static const std::set<std::string> expected_colors_ = {"red", "yellow", "green"};
  inline static const std::set<std::string> expected_arrows_ = {
    "up", "right", "left", "up_right", "up_left"};
};
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__TRAFFIC_LIGHT__LIGHT_BULB_TAGGING_HPP_
