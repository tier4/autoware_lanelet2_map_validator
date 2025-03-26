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

#include "lanelet2_map_validator/validators/lane/longitudinal_subtype_connection.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<LongitudinalSubtypeConnectionValidator> reg;
}

lanelet::validation::Issues LongitudinalSubtypeConnectionValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_longitudinal_subtype_connection(map));

  return issues;
}

lanelet::validation::Issues
LongitudinalSubtypeConnectionValidator::check_longitudinal_subtype_connection(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::traffic_rules::TrafficRulesPtr vehicle_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      "validator", lanelet::Participants::Vehicle);
  lanelet::routing::RoutingGraphUPtr vehicle_routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *vehicle_rules);

  lanelet::traffic_rules::TrafficRulesPtr pedestrian_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      "validator", lanelet::Participants::Pedestrian);
  lanelet::routing::RoutingGraphUPtr pedestrian_routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *pedestrian_rules);

  for (const lanelet::ConstLanelet & lane : map.laneletLayer) {
    const lanelet::ConstLanelets vehicle_successors = vehicle_routing_graph_ptr->following(lane);
    const lanelet::ConstLanelets pedestrian_successors =
      pedestrian_routing_graph_ptr->following(lane);

    lanelet::ConstLanelets successors;
    successors.reserve(vehicle_successors.size() + pedestrian_successors.size());
    successors.insert(successors.end(), vehicle_successors.begin(), vehicle_successors.end());
    successors.insert(successors.end(), pedestrian_successors.begin(), pedestrian_successors.end());

    std::unordered_set<ConstLanelet> unique_successors(successors.begin(), successors.end());

    if (unique_successors.empty()) {
      continue;
    }

    const std::string current_subtype = lane.attributeOr(lanelet::AttributeName::Subtype, "");
    if (current_subtype == "crosswalk" || current_subtype == "walkway") {
      issues.emplace_back(
        lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, lane.id(),
        append_issue_code_prefix(
          this->name(), 1, "A crosswalk or walkway type lanelet cannot have a successor lanelet."));
      continue;
    }

    for (const lanelet::ConstLanelet & successor : unique_successors) {
      if (successor.attributeOr(lanelet::AttributeName::Subtype, "") != current_subtype) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, lane.id(),
          append_issue_code_prefix(
            this->name(), 2, "A lanelet and its successor must have the same subtype."));
        break;
      }
    }
  }

  return issues;
}

namespace traffic_rules
{
namespace
{
lanelet::traffic_rules::RegisterTrafficRules<LongitudinalSubtypeConnectionValidationRules>
  vehicle_rules("validator", Participants::Vehicle);
lanelet::traffic_rules::RegisterTrafficRules<LongitudinalSubtypeConnectionValidationPedestrianRules>
  pedestrian_rules("validator", Participants::Pedestrian);
}  // namespace

Optional<bool> LongitudinalSubtypeConnectionValidationRules::canPass(
  const std::string & type, const std::string & /*location*/) const
{
  using ParticipantsMap = std::map<std::string, std::vector<std::string>>;
  using Value = AttributeValueString;
  // All lanelets will have a vehicle participant for this validation
  static const ParticipantsMap ParticipantMap{
    {"", {Participants::Vehicle}},
    {Value::Road, {Participants::Vehicle, Participants::Bicycle}},
    {"road_shoulder", {Participants::Vehicle, Participants::Bicycle, Participants::Pedestrian}},
    {"pedestrian_lane", {Participants::Vehicle, Participants::Bicycle, Participants::Pedestrian}},
    {Value::Highway, {Participants::Vehicle}},
    {Value::BicycleLane, {Participants::Vehicle, Participants::Bicycle}},
    {Value::PlayStreet, {Participants::Pedestrian, Participants::Bicycle, Participants::Vehicle}},
    {Value::EmergencyLane, {Participants::Vehicle, Participants::VehicleEmergency}},
    {Value::Exit, {Participants::Pedestrian, Participants::Bicycle, Participants::Vehicle}},
    {Value::Walkway, {Participants::Vehicle, Participants::Pedestrian}},
    {Value::Crosswalk, {Participants::Pedestrian}},
    {Value::Stairs, {Participants::Vehicle, Participants::Pedestrian}},
    {Value::SharedWalkway,
     {Participants::Vehicle, Participants::Pedestrian, Participants::Bicycle}}};
  auto participants = ParticipantMap.find(type);
  if (participants == ParticipantMap.end()) {
    return {};
  }
  return utils::anyOf(participants->second, [this](auto & participant) {
    return this->participant().compare(0, participant.size(), participant) == 0;
  });
}

Optional<bool> LongitudinalSubtypeConnectionValidationPedestrianRules::canPass(
  const std::string & type, const std::string & /*location*/) const
{
  using ParticipantsMap = std::map<std::string, std::vector<std::string>>;
  using Value = AttributeValueString;
  // All lanelets will have a vehicle participant for this validation
  static const ParticipantsMap ParticipantMap{
    {"", {Participants::Pedestrian}},
    {Value::Road, {Participants::Vehicle, Participants::Bicycle}},
    {"road_shoulder", {Participants::Vehicle, Participants::Bicycle, Participants::Pedestrian}},
    {"pedestrian_lane", {Participants::Vehicle, Participants::Bicycle, Participants::Pedestrian}},
    {Value::Highway, {Participants::Vehicle}},
    {Value::BicycleLane, {Participants::Vehicle, Participants::Bicycle}},
    {Value::PlayStreet, {Participants::Pedestrian, Participants::Bicycle, Participants::Vehicle}},
    {Value::EmergencyLane, {Participants::Vehicle, Participants::VehicleEmergency}},
    {Value::Exit, {Participants::Pedestrian, Participants::Bicycle, Participants::Vehicle}},
    {Value::Walkway, {Participants::Vehicle, Participants::Pedestrian}},
    {Value::Crosswalk, {Participants::Pedestrian}},
    {Value::Stairs, {Participants::Vehicle, Participants::Pedestrian}},
    {Value::SharedWalkway,
     {Participants::Vehicle, Participants::Pedestrian, Participants::Bicycle}}};
  auto participants = ParticipantMap.find(type);
  if (participants == ParticipantMap.end()) {
    return {};
  }
  return utils::anyOf(participants->second, [this](auto & participant) {
    return this->participant().compare(0, participant.size(), participant) == 0;
  });
}
}  // namespace traffic_rules
}  // namespace lanelet::autoware::validation
