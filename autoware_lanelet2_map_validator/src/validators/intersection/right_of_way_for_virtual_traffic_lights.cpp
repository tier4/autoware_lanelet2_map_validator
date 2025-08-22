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

#include "lanelet2_map_validator/validators/intersection/right_of_way_for_virtual_traffic_lights.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/BasicRegulatoryElements.h>
#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RightOfWayForVirtualTrafficLightsValidator> reg;
}

lanelet::validation::Issues RightOfWayForVirtualTrafficLightsValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(
    issues, check_right_of_way_for_virtual_traffic_lights(map));

  return issues;
}

lanelet::validation::Issues
RightOfWayForVirtualTrafficLightsValidator::check_right_of_way_for_virtual_traffic_lights(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  auto traffic_rules = lanelet::traffic_rules::TrafficRulesFactory::create(
    lanelet::Locations::Germany, lanelet::Participants::Vehicle);
  auto routing_graph = lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  for (const auto & lanelet : map.laneletLayer) {
    const auto virtual_traffic_light_elems =
      lanelet.regulatoryElementsAs<lanelet::autoware::VirtualTrafficLight>();

    if (virtual_traffic_light_elems.empty()) {
      continue;
    }

    const auto right_of_way_elems = lanelet.regulatoryElementsAs<lanelet::RightOfWay>();

    if (right_of_way_elems.empty()) {
      // Issue-001: Lanelet with virtual_traffic_light missing right_of_way reference
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), lanelet.id()));
      continue;
    }

    if (right_of_way_elems.size() > 1) {
      // Issue-002: Multiple right_of_way regulatory elements in the same lanelet
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), lanelet.id()));
      continue;
    }

    const auto right_of_way_elem = right_of_way_elems.front();

    bool is_set_as_right_of_way = false;
    const auto right_of_way_lanelets =
      right_of_way_elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::RightOfWay);

    if (right_of_way_lanelets.size() != 1) {
      // Issue-003: right_of_way regulatory element should have exactly one right_of_way role
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 3), right_of_way_elem->id()));
      continue;
    }

    for (const auto & row_lanelet : right_of_way_lanelets) {
      if (row_lanelet.id() == lanelet.id()) {
        is_set_as_right_of_way = true;
        break;
      }
    }

    if (!is_set_as_right_of_way) {
      // Issue-004: right_of_way regulatory element doesn't set this lanelet as right_of_way role
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 4), right_of_way_elem->id()));
      continue;
    }

    const auto yield_lanelets =
      right_of_way_elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::Yield);

    std::vector<lanelet::ConstLanelet> conflicting_lanelets;
    for (const auto & other_lanelet : map.laneletLayer) {
      if (other_lanelet.id() == lanelet.id()) {
        continue;
      }

      const auto relation = routing_graph->routingRelation(lanelet, other_lanelet);
      if (relation == lanelet::routing::RelationType::Conflicting) {
        conflicting_lanelets.push_back(other_lanelet);
      }
    }

    for (const auto & conflicting_lanelet : conflicting_lanelets) {
      bool is_set_as_yield = false;
      for (const auto & yield_lanelet : yield_lanelets) {
        if (yield_lanelet.id() == conflicting_lanelet.id()) {
          is_set_as_yield = true;
          break;
        }
      }

      if (!is_set_as_yield) {
        // Issue-005: Conflicting lanelet not set as yield role in right_of_way regulatory element
        std::map<std::string, std::string> reason_map;
        reason_map["conflicting_lanelet_id"] = std::to_string(conflicting_lanelet.id());
        issues.emplace_back(construct_issue_from_code(
          issue_code(this->name(), 5), right_of_way_elem->id(), reason_map));
      }
    }

    std::vector<lanelet::Id> actual_yield_ids;
    for (const auto & yield_lanelet : yield_lanelets) {
      actual_yield_ids.push_back(yield_lanelet.id());
    }

    std::vector<lanelet::Id> expected_yield_ids;
    for (const auto & conflicting_lanelet : conflicting_lanelets) {
      expected_yield_ids.push_back(conflicting_lanelet.id());
    }

    for (const auto & actual_id : actual_yield_ids) {
      if (
        std::find(expected_yield_ids.begin(), expected_yield_ids.end(), actual_id) ==
        expected_yield_ids.end()) {
        // Issue-006: Unnecessary yield relationship
        std::map<std::string, std::string> reason_map;
        reason_map["unnecessary_yield_to"] = std::to_string(actual_id);
        issues.emplace_back(construct_issue_from_code(
          issue_code(this->name(), 6), right_of_way_elem->id(), reason_map));
      }
    }
  }
  return issues;
}
}  // namespace lanelet::autoware::validation
