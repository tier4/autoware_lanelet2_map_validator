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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__TURN_SIGNAL_DISTANCE_OVERLAP_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__TURN_SIGNAL_DISTANCE_OVERLAP_HPP_

#include "lanelet2_map_validator/config_store.hpp"

#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <string>
#include <unordered_set>

namespace lanelet::autoware::validation
{
class TurnSignalDistanceOverlapValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name()
  {
    return "mapping.intersection.turn_signal_distance_overlap";
  }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

  TurnSignalDistanceOverlapValidator()
  {
    const auto parameters = ValidatorConfigStore::parameters()[name()];
    default_turn_signal_distance_ =
      get_parameter_or<double>(parameters, "default_turn_signal_distance", 15.0);
  }

private:
  lanelet::validation::Issues check_turn_signal_distance_overlap(const lanelet::LaneletMap & map);
  std::unordered_set<lanelet::Id> find_overlapping_lanelets(
    const ConstLanelet & lane, const lanelet::routing::RoutingGraphPtr & routing_graph_ptr,
    double distance_threshold);
  std::string set_to_string(std::unordered_set<lanelet::Id> & id_set);

  inline constexpr static const char turn_direction_tag_[] = "turn_direction";
  inline constexpr static const char turn_signal_distance_tag_[] = "turn_signal_distance";
  double default_turn_signal_distance_;
};
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__INTERSECTION__TURN_SIGNAL_DISTANCE_OVERLAP_HPP_
