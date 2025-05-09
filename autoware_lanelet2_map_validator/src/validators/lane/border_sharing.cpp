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

#include "lanelet2_map_validator/validators/lane/border_sharing.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry.hpp>

#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<BorderSharingValidator> reg;
}

lanelet::validation::Issues BorderSharingValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_border_sharing(map));

  return issues;
}

lanelet::validation::Issues BorderSharingValidator::check_border_sharing(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::traffic_rules::TrafficRulesPtr traffic_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      "validator", lanelet::Participants::Vehicle);
  lanelet::routing::RoutingGraphUPtr routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  std::vector<std::pair<lanelet::Id, lanelet::Id>> suspicious_pairs;
  for (const lanelet::ConstLanelet & current_lane : map.laneletLayer) {
    // Get the surrounding polygon of the lanelet
    const lanelet::BasicPolygon2d surrounding_polygon =
      expanded_lanelet_polygon(current_lane, 1.05);

    // Collect nearby lanelets which are candidates violating the requirements
    lanelet::BoundingBox2d bbox2d = lanelet::geometry::boundingBox2d(surrounding_polygon);
    lanelet::ConstLanelets nearby_lanelets = map.laneletLayer.search(bbox2d);

    // Collect suspicious pairs of lanelets violating the border sharing rule
    for (const auto & candidate_lane : nearby_lanelets) {
      // Ignore its own self
      if (current_lane.id() == candidate_lane.id()) {
        continue;
      }

      // If current and candidate have a proper relation, that's OK so skip it
      const lanelet::routing::RelationType relation =
        get_relation(routing_graph_ptr, current_lane, candidate_lane);
      if (
        relation != lanelet::routing::RelationType::Conflicting &&
        relation != lanelet::routing::RelationType::None) {
        continue;
      }

      // Assume that high IoU means pseudo-bidirectional lanelets
      if (
        relation == lanelet::routing::RelationType::Conflicting &&
        intersection_over_union(surrounding_polygon, candidate_lane.polygon2d().basicPolygon()) >
          iou_threshold_) {
        continue;
      }

      // Skip valid opposite lanes
      if (
        candidate_lane.rightBound() == current_lane.rightBound().invert() ||
        candidate_lane.leftBound() == current_lane.leftBound().invert()) {
        continue;
      }

      // If the surrounding_polygon covers the border of a non-related lanelet, that lanelet might
      // be suspicious
      if (
        boost::geometry::covered_by(
          candidate_lane.leftBound2d().basicLineString(), surrounding_polygon) ||
        boost::geometry::covered_by(
          candidate_lane.rightBound2d().basicLineString(), surrounding_polygon)) {
        suspicious_pairs.push_back({current_lane.id(), candidate_lane.id()});
      }
    }
  }

  // Sort out pairs by their directionality
  std::set<std::pair<lanelet::Id, lanelet::Id>> unidirectional_pairs;
  std::set<std::pair<lanelet::Id, lanelet::Id>> bidirectional_pairs;

  for (const auto & pair : suspicious_pairs) {
    std::pair<lanelet::Id, lanelet::Id> reversed_pair = {pair.second, pair.first};
    if (unidirectional_pairs.count(reversed_pair)) {
      bidirectional_pairs.insert(pair);
      unidirectional_pairs.erase(reversed_pair);
    } else {
      unidirectional_pairs.insert(pair);
    }
  }

  // Emplace issues violating vm-01-03
  // Output issues for both lanelets
  // It may be redundant but couldn't come up with a better way.

  std::map<std::string, std::string> substitution_map1;
  for (const auto & pair : bidirectional_pairs) {
    substitution_map1["lanelet_id"] = std::to_string(pair.second);
    issues.emplace_back(
      construct_issue_from_code(issue_code(this->name(), 1), pair.first, substitution_map1));
    substitution_map1["lanelet_id"] = std::to_string(pair.first);
    issues.emplace_back(
      construct_issue_from_code(issue_code(this->name(), 1), pair.second, substitution_map1));
  }

  // Emplace issues violating vm-01-04
  // Output issue only for the longest lanelet and show a list of lanelets
  // that the surrounding polygon covers
  std::map<std::string, std::string> substitution_map2;
  const auto ids_to_string = [](const lanelet::Ids & ids) {
    std::string result = "";
    for (const auto & id : ids) {
      result += std::to_string(id) + " ";
    }
    result.resize(result.size() - 1);
    return result;
  };
  for (auto it = unidirectional_pairs.begin(); it != unidirectional_pairs.end();) {
    lanelet::Id current_first = it->first;

    lanelet::Ids adjacents;
    while (it != unidirectional_pairs.end() && it->first == current_first) {
      adjacents.push_back(it->second);
      ++it;
    }

    if (adjacents.size() <= 0) {
      continue;
    }

    substitution_map2["lanelet_ids"] = ids_to_string(adjacents);

    issues.emplace_back(
      construct_issue_from_code(issue_code(this->name(), 2), current_first, substitution_map2));
  }

  return issues;
}

lanelet::BasicPolygon2d BorderSharingValidator::expanded_lanelet_polygon(
  const lanelet::ConstLanelet & lane, const double & scale_factor)
{
  lanelet::BasicPolygon2d result;

  lanelet::BasicLineString2d left_bound = lane.leftBound2d().basicLineString();
  lanelet::BasicLineString2d right_bound = lane.rightBound2d().invert().basicLineString();

  if (left_bound.size() < 2 || right_bound.size() < 2) {
    throw std::runtime_error("A lanelet has a bound less than two points.");
  }

  const double front_width =
    (left_bound.front() - right_bound.back()).norm();  // note that right is inverted
  const double back_width = (left_bound.back() - right_bound.front()).norm();
  const double delta = (front_width + back_width) / 2 * (scale_factor - 1);

  const auto unit_direction = [](lanelet::BasicPoint2d from, lanelet::BasicPoint2d to) {
    lanelet::BasicPoint2d direction = to - from;
    return (direction.norm() > 0.0) ? direction / direction.norm()
                                    : lanelet::BasicPoint2d(0.0, 0.0);
  };

  for (const auto & point : left_bound) {
    lanelet::ArcCoordinates arc_point = lanelet::geometry::toArcCoordinates(left_bound, point);
    arc_point.distance += delta;
    lanelet::BasicPoint2d extended_point =
      lanelet::geometry::fromArcCoordinates(left_bound, arc_point);

    if (point == left_bound.front()) {
      extended_point += delta * unit_direction(left_bound[1], left_bound[0]);
    }
    if (point == left_bound.back()) {
      extended_point +=
        delta *
        unit_direction(left_bound[left_bound.size() - 2], left_bound[left_bound.size() - 1]);
    }

    result.push_back(extended_point);
  }

  for (const auto & point : right_bound) {
    lanelet::ArcCoordinates arc_point = lanelet::geometry::toArcCoordinates(right_bound, point);
    arc_point.distance += delta;
    lanelet::BasicPoint2d extended_point =
      lanelet::geometry::fromArcCoordinates(right_bound, arc_point);

    if (point == right_bound.front()) {
      extended_point += delta * unit_direction(right_bound[1], right_bound[0]);
    }
    if (point == right_bound.back()) {
      extended_point +=
        delta *
        unit_direction(right_bound[right_bound.size() - 2], right_bound[right_bound.size() - 1]);
    }

    result.push_back(extended_point);
  }

  return result;
}

lanelet::routing::RelationType BorderSharingValidator::get_relation(
  const lanelet::routing::RoutingGraphUPtr & routing_graph_ptr, const lanelet::ConstLanelet from,
  const lanelet::ConstLanelet to)
{
  // This can get relations except "previous"
  const auto relation = routing_graph_ptr->routingRelation(from, to, true);
  if (!!relation) {
    return relation.get();
  }

  // Check previous
  const auto previous_relation = routing_graph_ptr->routingRelation(to, from, false);
  if (!!previous_relation && previous_relation.get() == lanelet::routing::RelationType::Successor) {
    return lanelet::routing::RelationType::Successor;
  }

  return lanelet::routing::RelationType::None;
}

double BorderSharingValidator::intersection_over_union(
  const lanelet::BasicPolygon2d & polygon1, const lanelet::BasicPolygon2d & polygon2)
{
  std::vector<lanelet::BasicPolygon2d> intersection;
  boost::geometry::intersection(polygon1, polygon2, intersection);
  double intersection_area = 0.0;
  for (const auto & portion : intersection) {
    intersection_area += boost::geometry::area(portion);
  }

  const double area1 = boost::geometry::area(polygon1);
  const double area2 = boost::geometry::area(polygon2);
  const double union_area = area1 + area2 - intersection_area;

  return intersection_area / union_area;
}

namespace traffic_rules
{
namespace
{
lanelet::traffic_rules::RegisterTrafficRules<BorderValidationRules> rules(
  "validator", Participants::Vehicle);
}

Optional<bool> BorderValidationRules::canPass(
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
    {Value::Crosswalk, {Participants::Vehicle, Participants::Pedestrian}},
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
