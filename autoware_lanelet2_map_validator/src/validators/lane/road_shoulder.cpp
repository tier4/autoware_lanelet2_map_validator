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

#include "lanelet2_map_validator/validators/lane/road_shoulder.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RoadShoulderValidator> reg;
}

lanelet::validation::Issues RoadShoulderValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_road_shoulder(map));

  return issues;
}

lanelet::validation::Issues RoadShoulderValidator::check_road_shoulder(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::traffic_rules::TrafficRulesPtr traffic_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      "validator", lanelet::Participants::Vehicle);

  lanelet::routing::RoutingGraphUPtr routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  for (const auto & lanelet : map.laneletLayer) {
    const auto & attrs = lanelet.attributes();
    const auto & type_it = attrs.find(lanelet::AttributeName::Type);
    const auto & subtype_it = attrs.find(lanelet::AttributeName::Subtype);

    if (
      type_it == attrs.end() || subtype_it == attrs.end() || type_it->second != "lanelet" ||
      subtype_it->second != "road_shoulder") {
      continue;
    }

    const auto left_lanelets = routing_graph_ptr->adjacentLeft(lanelet);
    const auto right_lanelets = routing_graph_ptr->adjacentRight(lanelet);
    const bool has_left = left_lanelets.has_value();
    const bool has_right = right_lanelets.has_value();

    // Issue-001: Road shoulder must have at least one adjacent lanelet
    if (!has_left && !has_right) {
      issues.push_back(construct_issue_from_code(issue_code(this->name(), 1), lanelet.id()));
      continue;
    }

    if (has_left != has_right) {
      // Issue-002: Check if adjacent lanelet is a road
      const auto & adjacent_lanelets = has_left ? left_lanelets : right_lanelets;
      const auto & adjacent = adjacent_lanelets.value();
      const auto & adj_attrs = adjacent.attributes();
      const auto & adj_subtype_it = adj_attrs.find(lanelet::AttributeName::Subtype);

      if (adj_subtype_it == adj_attrs.end() || adj_subtype_it->second != "road") {
        issues.push_back(construct_issue_from_code(issue_code(this->name(), 2), lanelet.id()));
      }

      // Issue-003: Check if empty side bound is road_border
      const auto & empty_side_bound = has_left ? lanelet.rightBound() : lanelet.leftBound();
      const auto & bound_attrs = empty_side_bound.attributes();
      const auto & bound_type_it = bound_attrs.find(lanelet::AttributeName::Type);

      if (bound_type_it == bound_attrs.end() || bound_type_it->second != "road_border") {
        issues.push_back(
          construct_issue_from_code(issue_code(this->name(), 3), empty_side_bound.id()));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
