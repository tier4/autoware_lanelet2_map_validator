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

#include "lanelet2_map_validator/validators/intersection/turn_signal_distance_overlap.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <deque>
#include <map>
#include <string>
#include <unordered_set>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<TurnSignalDistanceOverlapValidator> reg;
}

lanelet::validation::Issues TurnSignalDistanceOverlapValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_turn_signal_distance_overlap(map));

  return issues;
}

lanelet::validation::Issues TurnSignalDistanceOverlapValidator::check_turn_signal_distance_overlap(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::traffic_rules::TrafficRulesPtr traffic_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      lanelet::Locations::Germany, lanelet::Participants::Vehicle);
  lanelet::routing::RoutingGraphPtr routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  for (const auto & lane : map.laneletLayer) {
    if (
      !lane.hasAttribute(turn_direction_tag_) ||
      lane.attribute(turn_direction_tag_).value() == "straight") {
      continue;
    }

    const double threshold =
      lane.attributeOr(turn_signal_distance_tag_, default_turn_signal_distance_);
    auto overlapping_lanelets = find_overlapping_lanelets(lane, routing_graph_ptr, threshold);

    if (overlapping_lanelets.empty()) {
      continue;
    }

    std::map<std::string, std::string> prev_ids_map;
    prev_ids_map["prev_ids"] = set_to_string(overlapping_lanelets);
    issues.emplace_back(
      construct_issue_from_code(issue_code(this->name(), 1), lane.id(), prev_ids_map));
  }

  return issues;
}

std::unordered_set<lanelet::Id> TurnSignalDistanceOverlapValidator::find_overlapping_lanelets(
  const ConstLanelet & intersection_lane,
  const lanelet::routing::RoutingGraphPtr & routing_graph_ptr, double distance_threshold)
{
  std::unordered_set<lanelet::Id> result;
  std::unordered_set<lanelet::Id> visited;
  std::deque<std::pair<lanelet::ConstLanelet, double>> queue;

  for (const auto & prev : routing_graph_ptr->previous(intersection_lane)) {
    queue.emplace_back(prev, calc_lanelet_length(prev));
  }

  while (!queue.empty()) {
    auto [current, cum_length] = queue.front();
    queue.pop_front();

    if (visited.count(current.id()) > 0) {
      continue;
    }

    if (cum_length > distance_threshold) {
      continue;
    }

    visited.insert(current.id());

    if (current.hasAttribute(turn_direction_tag_)) {
      if (
        (intersection_lane.attribute(turn_direction_tag_).value() == "left" &&
         current.attribute(turn_direction_tag_).value() == "right") ||
        (intersection_lane.attribute(turn_direction_tag_).value() == "right" &&
         current.attribute(turn_direction_tag_).value() == "left")) {
        result.insert(current.id());
      }
    }

    for (const auto & prev : routing_graph_ptr->previous(current)) {
      queue.emplace_back(prev, cum_length + calc_lanelet_length(prev));
    }
  }

  return result;
}

std::string TurnSignalDistanceOverlapValidator::set_to_string(
  std::unordered_set<lanelet::Id> & id_set)
{
  std::string result = "";
  for (auto it = id_set.begin(); it != id_set.end(); ++it) {
    if (it != id_set.begin()) {
      result += ",";
    }
    result += std::to_string(*it);
  }
  return result;
}

double TurnSignalDistanceOverlapValidator::calc_lanelet_length(const lanelet::ConstLanelet & lane)
{
  return static_cast<double>(boost::geometry::length(lanelet.centerline().basicLineString()));
}

}  // namespace lanelet::autoware::validation
