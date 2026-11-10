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

#include "lanelet2_map_validator/validators/lane/border_sharing.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry.hpp>

#include <lanelet2_core/Attribute.h>
#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <algorithm>
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
  lanelet::autoware::validation::appendIssues(issues, check_fence_on_shared_border(map));
  lanelet::autoware::validation::appendIssues(issues, check_fence_typed_shared_border(map));

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

  // A fence lies on the border if the border itself is a fence or a fence is at the same position
  // as at least one segment of the border
  const FenceSegments fence_segments = collect_fence_segments(map);
  const auto has_fence = [&fence_segments](const lanelet::ConstLineString3d & line) {
    return is_fence(line) || fence_lies_on(line, fence_segments);
  };

  std::vector<std::pair<lanelet::Id, lanelet::Id>> suspicious_pairs;
  LaneletPairs fence_separated_pairs;
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
      const bool left_covered = boost::geometry::covered_by(
        candidate_lane.leftBound2d().basicLineString(), surrounding_polygon);
      const bool right_covered = boost::geometry::covered_by(
        candidate_lane.rightBound2d().basicLineString(), surrounding_polygon);
      if (!left_covered && !right_covered) {
        continue;
      }
      suspicious_pairs.push_back({current_lane.id(), candidate_lane.id()});

      if (
        (left_covered && has_fence(candidate_lane.leftBound())) ||
        (right_covered && has_fence(candidate_lane.rightBound()))) {
        fence_separated_pairs.insert(std::minmax(current_lane.id(), candidate_lane.id()));
      }
    }
  }

  // Lanelets separated by a fence don't have to share their border
  suspicious_pairs.erase(
    std::remove_if(
      suspicious_pairs.begin(), suspicious_pairs.end(),
      [&fence_separated_pairs](const std::pair<lanelet::Id, lanelet::Id> & pair) {
        return fence_separated_pairs.count(std::minmax(pair.first, pair.second)) > 0;
      }),
    suspicious_pairs.end());

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

lanelet::validation::Issues BorderSharingValidator::check_fence_on_shared_border(
  const lanelet::LaneletMap & map)
{
  const FenceSegments fence_segments = collect_fence_segments(map);

  LaneletPairs pairs;
  for (const auto & line : map.lineStringLayer) {
    if (!fence_lies_on(line, fence_segments)) {
      continue;
    }
    const LaneletPairs sharing_pairs = find_lanelets_sharing(map, line);
    pairs.insert(sharing_pairs.begin(), sharing_pairs.end());
  }

  return issues_for_pairs(pairs, 3);
}

lanelet::validation::Issues BorderSharingValidator::check_fence_typed_shared_border(
  const lanelet::LaneletMap & map)
{
  LaneletPairs pairs;
  for (const auto & line : map.lineStringLayer) {
    if (!is_fence(line)) {
      continue;
    }
    const LaneletPairs sharing_pairs = find_lanelets_sharing(map, line);
    pairs.insert(sharing_pairs.begin(), sharing_pairs.end());
  }

  return issues_for_pairs(pairs, 4);
}

BorderSharingValidator::LaneletPairs BorderSharingValidator::find_lanelets_sharing(
  const lanelet::LaneletMap & map, const lanelet::ConstLineString3d & line)
{
  if (line.size() < 2) {
    return {};
  }

  // findUsages() distinguishes the direction of the linestring, so both directions are needed
  struct Usage
  {
    lanelet::ConstLanelet lanelet;
    bool lanelet_is_on_left;  // true if the lanelet lies on the left side of the linestring
  };
  std::vector<Usage> usages;
  for (const lanelet::ConstLineString3d & direction : {line, line.invert()}) {
    for (const auto & lane : map.laneletLayer.findUsages(direction)) {
      const bool is_left_bound = lane.leftBound().id() == line.id();
      const bool is_right_bound = lane.rightBound().id() == line.id();
      if (is_left_bound == is_right_bound) {
        continue;
      }
      // A lanelet lies on the left side of its right bound. The side flips if the bound is
      // inverted from the linestring.
      const lanelet::ConstLineString3d bound =
        is_right_bound ? lane.rightBound() : lane.leftBound();
      usages.push_back({lane, is_right_bound != (bound.inverted() != line.inverted())});
    }
  }

  const auto is_road = [](const lanelet::ConstLanelet & lane) {
    return std::string(lane.attributeOr(lanelet::AttributeName::Subtype, "")) ==
           lanelet::AttributeValueString::Road;
  };

  LaneletPairs pairs;
  for (size_t i = 0; i < usages.size(); i++) {
    for (size_t j = i + 1; j < usages.size(); j++) {
      // Only pairs including a road lanelet affect the planning of Autoware
      if (!is_road(usages[i].lanelet) && !is_road(usages[j].lanelet)) {
        continue;
      }

      // Lanelets on the same side of the shared border are overlapping, not adjacent
      if (usages[i].lanelet_is_on_left == usages[j].lanelet_is_on_left) {
        continue;
      }

      pairs.insert(std::minmax(usages[i].lanelet.id(), usages[j].lanelet.id()));
    }
  }

  return pairs;
}

lanelet::validation::Issues BorderSharingValidator::issues_for_pairs(
  const LaneletPairs & pairs, const int issue_number)
{
  lanelet::validation::Issues issues;

  // Output issues for both lanelets of each pair
  std::map<std::string, std::string> substitution_map;
  for (const auto & [id1, id2] : pairs) {
    substitution_map["lanelet_id"] = std::to_string(id2);
    issues.emplace_back(
      construct_issue_from_code(issue_code(this->name(), issue_number), id1, substitution_map));
    substitution_map["lanelet_id"] = std::to_string(id1);
    issues.emplace_back(
      construct_issue_from_code(issue_code(this->name(), issue_number), id2, substitution_map));
  }

  return issues;
}

BorderSharingValidator::FenceSegments BorderSharingValidator::collect_fence_segments(
  const lanelet::LaneletMap & map)
{
  FenceSegments fence_segments;
  for (const auto & line : map.lineStringLayer) {
    if (!is_fence(line)) {
      continue;
    }
    for (size_t i = 0; i + 1 < line.size(); i++) {
      fence_segments[make_segment(line[i], line[i + 1])].insert(line.id());
    }
  }
  return fence_segments;
}

bool BorderSharingValidator::is_fence(const lanelet::ConstLineString3d & line)
{
  return std::string(line.attributeOr(lanelet::AttributeName::Type, "")) ==
         lanelet::AttributeValueString::Fence;
}

bool BorderSharingValidator::fence_lies_on(
  const lanelet::ConstLineString3d & line, const FenceSegments & fence_segments)
{
  for (size_t i = 0; i + 1 < line.size(); i++) {
    const auto it = fence_segments.find(make_segment(line[i], line[i + 1]));
    if (it == fence_segments.end()) {
      continue;
    }
    // The linestring itself doesn't count even if its type is fence
    if (it->second.size() > 1 || it->second.count(line.id()) == 0) {
      return true;
    }
  }
  return false;
}

BorderSharingValidator::Segment BorderSharingValidator::make_segment(
  const lanelet::ConstPoint3d & a, const lanelet::ConstPoint3d & b)
{
  const Point point_a{a.x(), a.y(), a.z()};
  const Point point_b{b.x(), b.y(), b.z()};
  return point_a < point_b ? Segment{point_a, point_b} : Segment{point_b, point_a};
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
