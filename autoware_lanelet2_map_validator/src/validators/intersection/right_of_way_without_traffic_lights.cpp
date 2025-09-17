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

#include "lanelet2_map_validator/validators/intersection/right_of_way_without_traffic_lights.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/BasicRegulatoryElements.h>
#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RightOfWayWithoutTrafficLightsValidator> reg;
}

lanelet::validation::Issues RightOfWayWithoutTrafficLightsValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(
    issues, check_right_of_way_without_traffic_lights(map));

  return issues;
}

lanelet::validation::Issues
RightOfWayWithoutTrafficLightsValidator::check_right_of_way_without_traffic_lights(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  auto traffic_rules = lanelet::traffic_rules::TrafficRulesFactory::create(
    lanelet::Locations::Germany, lanelet::Participants::Vehicle);
  auto routing_graph = lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  for (const lanelet::ConstLanelet & lanelet : map.laneletLayer) {
    if (!lanelet.hasAttribute("turn_direction")) {
      continue;
    }

    auto traffic_light_elems = lanelet.regulatoryElementsAs<lanelet::TrafficLight>();
    if (!traffic_light_elems.empty()) {
      continue;
    }

    auto virtual_traffic_light_elems =
      lanelet.regulatoryElementsAs<lanelet::autoware::VirtualTrafficLight>();
    if (!virtual_traffic_light_elems.empty()) {
      continue;
    }

    const auto right_of_way_elems = lanelet.regulatoryElementsAs<lanelet::RightOfWay>();

    // Process each right_of_way regulatory element (multiple are allowed)
    for (const auto & right_of_way_elem : right_of_way_elems) {
      auto right_of_way_lanelets =
        right_of_way_elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::RightOfWay);

      // Issue-002: right_of_way regulatory element should have exactly one right_of_way role
      if (right_of_way_lanelets.size() != 1) {
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 1), right_of_way_elem->id()));
        continue;
      }

      bool is_set_as_right_of_way = false;
      for (const auto & param : right_of_way_lanelets) {
        if (param.id() == lanelet.id()) {
          is_set_as_right_of_way = true;
          break;
        }
      }

      // Issue-003: right_of_way regulatory element doesn't set this lanelet as right_of_way role
      if (!is_set_as_right_of_way) {
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 2), right_of_way_elem->id()));
        continue;
      }

      auto yield_lanelets =
        right_of_way_elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::Yield);

      std::vector<lanelet::ConstLanelet> conflicting_lanelets;
      for (const auto & other_lanelet : map.laneletLayer) {
        if (other_lanelet.id() == lanelet.id()) {
          continue;
        }

        const auto relation = routing_graph->routingRelation(lanelet, other_lanelet);
        if (relation == lanelet::routing::RelationType::Conflicting) {
          bool has_same_source = false;

          auto prev_lanelets_current = routing_graph->previous(lanelet);
          auto prev_lanelets_other = routing_graph->previous(other_lanelet);

          for (const auto & prev_current : prev_lanelets_current) {
            for (const auto & prev_other : prev_lanelets_other) {
              if (prev_current.id() == prev_other.id()) {
                has_same_source = true;
                break;
              }
            }
            if (has_same_source) break;
          }

          if (!has_same_source) {
            conflicting_lanelets.push_back(other_lanelet);
          }
        }
      }

      std::string turn_direction = lanelet.attribute("turn_direction").value();

      std::set<lanelet::Id> conflicting_ids;
      for (const auto & conflicting_lanelet : conflicting_lanelets) {
        conflicting_ids.insert(conflicting_lanelet.id());
      }

      std::set<lanelet::Id> yield_ids;
      for (const auto & yield_lanelet : yield_lanelets) {
        yield_ids.insert(yield_lanelet.id());
      }

      // Issue-004: Check yield relationships, all lanelets with right_of_way regulatory elements
      // are treated as priority lanes that should yield to all conflicting lanes
      std::set<lanelet::Id> missing_yields;
      std::set_difference(
        conflicting_ids.begin(), conflicting_ids.end(), yield_ids.begin(), yield_ids.end(),
        std::inserter(missing_yields, missing_yields.begin()));

      for (const auto & missing_id : missing_yields) {
        std::map<std::string, std::string> reason_map;
        reason_map["conflicting_lanelet_id"] = std::to_string(missing_id);
        reason_map["turn_direction"] = turn_direction;
        reason_map["lane_type"] = "priority";
        issues.emplace_back(construct_issue_from_code(
          issue_code(this->name(), 3), right_of_way_elem->id(), reason_map));
      }

      // Issue-005: Check for unnecessary yield relationships
      std::set<lanelet::Id> unnecessary_yields;
      std::set_difference(
        yield_ids.begin(), yield_ids.end(), conflicting_ids.begin(), conflicting_ids.end(),
        std::inserter(unnecessary_yields, unnecessary_yields.begin()));

      for (const auto & unnecessary_id : unnecessary_yields) {
        std::map<std::string, std::string> reason_map;
        reason_map["unnecessary_yield_to"] = std::to_string(unnecessary_id);
        reason_map["turn_direction"] = lanelet.attribute("turn_direction").value();
        issues.emplace_back(construct_issue_from_code(
          issue_code(this->name(), 4), right_of_way_elem->id(), reason_map));
      }
    }
  }
  return issues;
}

}  // namespace lanelet::autoware::validation
