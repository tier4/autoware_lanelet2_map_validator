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

#include "lanelet2_map_validator/validators/intersection/lanelet_division.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <map>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<LaneletDivisionValidator> reg;
}

lanelet::validation::Issues LaneletDivisionValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_lanelet_division(map));

  return issues;
}

lanelet::validation::Issues LaneletDivisionValidator::check_lanelet_division(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::traffic_rules::TrafficRulesPtr traffic_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      "validator", lanelet::Participants::Vehicle);
  lanelet::routing::RoutingGraphUPtr routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  for (const lanelet::ConstLanelet & lanelet : map.laneletLayer) {
    lanelet::Id current_intersection_area_id =
      lanelet.attributeOr("intersection_area", lanelet::InvalId);

    if (current_intersection_area_id == lanelet::InvalId) {
      continue;
    }

    const lanelet::ConstLanelets successors = routing_graph_ptr->following(lanelet);

    for (const lanelet::ConstLanelet & successor : successors) {
      lanelet::Id successor_intersection_area_id =
        successor.attributeOr("intersection_area", lanelet::InvalId);

      // If successor also has intersection_area tag with same value, this is an issue
      if (successor_intersection_area_id == current_intersection_area_id) {
        std::map<std::string, std::string> substitution_map;
        substitution_map["successor_id"] = std::to_string(successor.id());
        substitution_map["intersection_area_id"] = std::to_string(current_intersection_area_id);

        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 1), lanelet.id(), substitution_map));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
