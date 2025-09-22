// Copyright 2024 Autoware Foundation
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

#include "lanelet2_map_validator/validators/intersection/right_of_way_with_traffic_lights.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_core/primitives/BasicRegulatoryElements.h>
#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RightOfWayWithTrafficLightsValidator> reg;

std::optional<lanelet::ConstLineString3d> get_traffic_light_linestring(
  const lanelet::ConstLanelet & assigned_lanelet)
{
  const auto tl_reg_elems = assigned_lanelet.regulatoryElementsAs<lanelet::TrafficLight>();
  if (tl_reg_elems.empty()) {
    return std::nullopt;
  }

  const auto & tl_reg_elem = tl_reg_elems.front();
  const auto refers =
    tl_reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers);
  if (refers.empty()) {
    return std::nullopt;
  }
  return refers.front();
}

bool is_different_signal_timing(
  const lanelet::ConstLanelet & lane1, const lanelet::ConstLanelet & lane2,
  double perpendicular_threshold = M_PI / 4)  // 45 degrees threshold
{
  auto tl1_linestring = get_traffic_light_linestring(lane1);
  auto tl2_linestring = get_traffic_light_linestring(lane2);

  if (!tl1_linestring || !tl2_linestring) {
    return true;
  }

  const auto & tl1_start = tl1_linestring->front();
  const auto & tl1_end = tl1_linestring->back();
  double dx1 = tl1_end.x() - tl1_start.x();
  double dy1 = tl1_end.y() - tl1_start.y();

  const auto & tl2_start = tl2_linestring->front();
  const auto & tl2_end = tl2_linestring->back();
  double dx2 = tl2_end.x() - tl2_start.x();
  double dy2 = tl2_end.y() - tl2_start.y();

  double mag1 = std::hypot(dx1, dy1);
  double mag2 = std::hypot(dx2, dy2);

  if (mag1 < 1e-6 || mag2 < 1e-6) {
    return true;
  }

  double dot_product = (dx1 * dx2 + dy1 * dy2) / (mag1 * mag2);

  dot_product = std::abs(dot_product);
  dot_product = std::min(1.0, dot_product);

  if (dot_product > std::cos(perpendicular_threshold)) {
    return false;  // same signal timing (opposing directions)
  }

  return true;
}
}  // namespace

lanelet::validation::Issues RightOfWayWithTrafficLightsValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_right_of_way_with_traffic_lights(map));

  return issues;
}

lanelet::validation::Issues
RightOfWayWithTrafficLightsValidator::check_right_of_way_with_traffic_lights(
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

    auto turn_direction = lanelet.attribute("turn_direction").value();
    if (turn_direction != "left" && turn_direction != "right") {
      continue;
    }

    if (lanelet.regulatoryElementsAs<lanelet::TrafficLight>().empty()) {
      continue;
    }

    const auto right_of_way_elems = lanelet.regulatoryElementsAs<lanelet::RightOfWay>();

    if (right_of_way_elems.empty()) {
      // Issue-001: Lanelet with turn_direction and traffic_light reference missing right_of_way
      // reference
      std::map<std::string, std::string> reason_map;
      reason_map["turn_direction"] = lanelet.attribute("turn_direction").value();
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 1), lanelet.id(), reason_map));
      continue;
    } else if (right_of_way_elems.size() > 1) {
      // issue-002: Multiple right_of_way regulatory element in the same lanelet
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), lanelet.id()));
      continue;
    }

    lanelet::RegulatoryElementConstPtr right_of_way_elem = right_of_way_elems.front();
    bool is_set_as_right_of_way = false;
    auto right_of_way_lanelets =
      right_of_way_elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::RightOfWay);
    for (const auto & param : right_of_way_lanelets) {
      if (param.id() == lanelet.id()) {
        is_set_as_right_of_way = true;
        break;
      }
    }

    if (!is_set_as_right_of_way) {
      // issue-003: right_of_way regulatory element doesn't set this lanelet as right_of_way role
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), 3), right_of_way_elem->id()));
      continue;
    }
    auto yield_lanelets =
      right_of_way_elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::Yield);

    auto conflicting_lanelets = routing_graph->conflicting(lanelet);

    std::set<lanelet::Id> expected_yield_ids;

    // Cache previous lanelets for current lanelet to avoid repeated lookups
    auto current_previous = routing_graph->previous(lanelet);
    std::set<lanelet::Id> current_prev_ids;
    for (const auto & prev : current_previous) {
      current_prev_ids.insert(prev.id());
    }

    for (const auto & conflicting_lanelet : conflicting_lanelets) {
      // Rule 0: check if lanelets are going in the same direction (no yield needed)
      bool same_direction = false;

      // check if they have the same previous lanelet (coming from same source)
      // TODO(MRADITYA01): Use same_source utils after utils are merged

      auto conflicting_opt = *conflicting_lanelet.lanelet();
      auto other_previous = routing_graph->previous(conflicting_opt);

      // Check for any common previous lanelet
      for (const auto & other_prev : other_previous) {
        if (current_prev_ids.find(other_prev.id()) != current_prev_ids.end()) {
          same_direction = true;
          break;
        }
      }

      // if same direction, skip yield logic
      if (same_direction) {
        continue;
      }

      bool should_yield = false;

      // Rule 1: Yield to conflicting lanelets that have different signal timing
      if (is_different_signal_timing(lanelet, conflicting_opt)) {
        should_yield = true;
      }

      // Rule 2: If vehicle is turning left, yield to opposing right-turn lanes
      if (turn_direction == "left" && conflicting_opt.hasAttribute("turn_direction")) {
        auto other_turn_direction = conflicting_opt.attribute("turn_direction").value();
        if (other_turn_direction == "right") {
          should_yield = true;
        }
      }

      if (should_yield) {
        expected_yield_ids.insert(conflicting_opt.id());
      }
    }

    std::set<lanelet::Id> actual_yield_ids;
    for (const auto & yield_lanelet : yield_lanelets) {
      actual_yield_ids.insert(yield_lanelet.id());
    }

    // find missing yield relationships (in expected but not in actual)
    std::set<lanelet::Id> missing_yields;
    std::set_difference(
      expected_yield_ids.begin(), expected_yield_ids.end(), actual_yield_ids.begin(),
      actual_yield_ids.end(), std::inserter(missing_yields, missing_yields.begin()));

    for (const auto & missing_id : missing_yields) {
      // issue-004: Missing required yield relationship
      std::map<std::string, std::string> reason_map;
      reason_map["missing_yield_to"] = std::to_string(missing_id);
      reason_map["turn_direction"] = turn_direction;
      issues.emplace_back(construct_issue_from_code(
        issue_code(this->name(), 4), right_of_way_elem->id(), reason_map));
    }

    // find unnecessary yield relationships (in actual but not in expected)
    std::set<lanelet::Id> unnecessary_yields;
    std::set_difference(
      actual_yield_ids.begin(), actual_yield_ids.end(), expected_yield_ids.begin(),
      expected_yield_ids.end(), std::inserter(unnecessary_yields, unnecessary_yields.begin()));

    for (const auto & unnecessary_id : unnecessary_yields) {
      // issue-005: Unnecessary yield relationship
      std::map<std::string, std::string> reason_map;
      reason_map["unnecessary_yield_to"] = std::to_string(unnecessary_id);
      reason_map["turn_direction"] = turn_direction;
      issues.emplace_back(construct_issue_from_code(
        issue_code(this->name(), 5), right_of_way_elem->id(), reason_map));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
