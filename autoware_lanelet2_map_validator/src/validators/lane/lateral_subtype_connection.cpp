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

#include "lanelet2_map_validator/validators/lane/lateral_subtype_connection.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/LaneletMap.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <map>
#include <set>
#include <string>
#include <utility>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<LateralSubtypeConnectionValidator> reg;

void check_adjacent_subtype_compatibility(
  const lanelet::ConstLanelet & current_lane, const lanelet::ConstLanelet & adjacent_lane,
  std::set<std::pair<lanelet::Id, lanelet::Id>> & processed_pairs,
  lanelet::validation::Issues & issues, const std::string & validator_name)
{
  std::pair<lanelet::Id, lanelet::Id> current_pair = {current_lane.id(), adjacent_lane.id()};
  std::pair<lanelet::Id, lanelet::Id> reverse_pair = {adjacent_lane.id(), current_lane.id()};
  if (processed_pairs.count(current_pair) || processed_pairs.count(reverse_pair)) {
    return;
  }
  processed_pairs.insert(current_pair);

  const std::string current_subtype = current_lane.attributeOr(lanelet::AttributeName::Subtype, "");
  const std::string adjacent_subtype =
    adjacent_lane.attributeOr(lanelet::AttributeName::Subtype, "");

  std::string norm_current = current_subtype;
  std::string norm_adjacent = adjacent_subtype;

  std::set<std::string> vehicle_suitable = {
    "road", "road_shoulder", "pedestrian_lane", "bicycle_lane"};

  bool current_vehicle = vehicle_suitable.count(norm_current) > 0;
  bool adjacent_vehicle = vehicle_suitable.count(norm_adjacent) > 0;

  if (!current_vehicle || !adjacent_vehicle) {
    std::map<std::string, std::string> substitution_map;
    substitution_map["adjacent_lanelet_id"] = std::to_string(adjacent_lane.id());
    substitution_map["adjacent_subtype"] = adjacent_subtype;
    substitution_map["self_adjacent_subtype"] = current_subtype;
    issues.emplace_back(construct_issue_from_code(
      issue_code(validator_name, 1), current_lane.id(), substitution_map));
  }
}
}  // namespace

lanelet::validation::Issues LateralSubtypeConnectionValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_lateral_subtype_connection(map));

  return issues;
}

lanelet::validation::Issues LateralSubtypeConnectionValidator::check_lateral_subtype_connection(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::traffic_rules::TrafficRulesPtr traffic_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      "validator", lanelet::Participants::Vehicle);
  lanelet::routing::RoutingGraphUPtr routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  std::set<std::pair<lanelet::Id, lanelet::Id>> processed_pairs;

  for (const lanelet::ConstLanelet & lane : map.laneletLayer) {
    const auto left_adjacent = routing_graph_ptr->adjacentLeft(lane);
    if (left_adjacent) {
      check_adjacent_subtype_compatibility(
        lane, left_adjacent.get(), processed_pairs, issues, this->name());
    }

    const auto right_adjacent = routing_graph_ptr->adjacentRight(lane);
    if (right_adjacent) {
      check_adjacent_subtype_compatibility(
        lane, right_adjacent.get(), processed_pairs, issues, this->name());
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
