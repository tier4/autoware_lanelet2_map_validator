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

#include <lanelet2_core/Attribute.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <algorithm>
#include <map>
#include <optional>
#include <set>
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

  const auto to_points = [](const lanelet::ConstLineString3d & line) {
    std::vector<Point> points;
    points.reserve(line.size());
    for (const auto & point : line) {
      points.push_back({point.x(), point.y(), point.z()});
    }
    return points;
  };

  // Collect the segments of fences so that a fence at the same position as a bound can be found
  std::set<Segment> fence_segments;
  for (const auto & line : map.lineStringLayer) {
    if (
      std::string(line.attributeOr(lanelet::AttributeName::Type, "")) !=
      lanelet::AttributeValueString::Fence) {
      continue;
    }
    const auto points = to_points(line);
    for (size_t i = 0; i + 1 < points.size(); i++) {
      fence_segments.insert(make_segment(points[i], points[i + 1]));
    }
  }

  // Group the bounds by their end points. Bounds at the same position always fall in the same group
  std::map<Segment, std::vector<Bound>> bounds_by_ends;
  for (const auto & lane : map.laneletLayer) {
    for (const bool is_right_bound : {false, true}) {
      const lanelet::ConstLineString3d line = is_right_bound ? lane.rightBound() : lane.leftBound();
      if (line.size() < 2) {
        continue;
      }
      Bound bound{lane, line, to_points(line), is_right_bound};
      bounds_by_ends[make_segment(bound.points.front(), bound.points.back())].push_back(bound);
    }
  }

  const auto is_road = [](const lanelet::ConstLanelet & lane) {
    return std::string(lane.attributeOr(lanelet::AttributeName::Subtype, "")) ==
           lanelet::AttributeValueString::Road;
  };

  std::set<std::pair<lanelet::Id, lanelet::Id>> unshared_pairs;
  std::set<std::pair<lanelet::Id, lanelet::Id>> fence_shared_pairs;

  for (const auto & [ends, bounds] : bounds_by_ends) {
    for (size_t i = 0; i < bounds.size(); i++) {
      for (size_t j = i + 1; j < bounds.size(); j++) {
        const Bound & bound1 = bounds[i];
        const Bound & bound2 = bounds[j];

        if (bound1.lanelet.id() == bound2.lanelet.id()) {
          continue;
        }

        // Only pairs including a road lanelet affect the planning of Autoware
        if (!is_road(bound1.lanelet) && !is_road(bound2.lanelet)) {
          continue;
        }

        // The pair is adjacent only if the bounds are at the same position from end to end
        const std::optional<bool> reversed = same_position(bound1.points, bound2.points);
        if (!reversed) {
          continue;
        }

        // Lanelets on the same side of the bound are overlapping, not adjacent
        const bool lanelet2_is_on_left =
          *reversed ? !bound2.lanelet_is_on_left : bound2.lanelet_is_on_left;
        if (bound1.lanelet_is_on_left == lanelet2_is_on_left) {
          continue;
        }

        const bool is_shared = bound1.line.id() == bound2.line.id();
        const bool has_fence =
          has_fence_on(bound1, fence_segments) || has_fence_on(bound2, fence_segments);
        const lanelet::Id id1 = bound1.lanelet.id();
        const lanelet::Id id2 = bound2.lanelet.id();
        const std::pair<lanelet::Id, lanelet::Id> pair = std::minmax(id1, id2);

        if (has_fence && is_shared) {
          fence_shared_pairs.insert(pair);
        } else if (!has_fence && !is_shared) {
          unshared_pairs.insert(pair);
        }
      }
    }
  }

  // Output issues for both lanelets of each pair
  const auto emit_issues = [&](const auto & pairs, const int issue_number) {
    std::map<std::string, std::string> substitution_map;
    for (const auto & [id1, id2] : pairs) {
      substitution_map["lanelet_id"] = std::to_string(id2);
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), issue_number), id1, substitution_map));
      substitution_map["lanelet_id"] = std::to_string(id1);
      issues.emplace_back(
        construct_issue_from_code(issue_code(this->name(), issue_number), id2, substitution_map));
    }
  };
  emit_issues(unshared_pairs, 1);
  emit_issues(fence_shared_pairs, 3);

  return issues;
}

std::optional<bool> BorderSharingValidator::same_position(
  const std::vector<Point> & points1, const std::vector<Point> & points2)
{
  if (points1.size() != points2.size()) {
    return std::nullopt;
  }
  if (std::equal(points1.begin(), points1.end(), points2.begin())) {
    return false;
  }
  if (std::equal(points1.begin(), points1.end(), points2.rbegin())) {
    return true;
  }
  return std::nullopt;
}

bool BorderSharingValidator::has_fence_on(
  const Bound & bound, const std::set<Segment> & fence_segments)
{
  for (size_t i = 0; i + 1 < bound.points.size(); i++) {
    if (fence_segments.count(make_segment(bound.points[i], bound.points[i + 1]))) {
      return true;
    }
  }
  return false;
}

BorderSharingValidator::Segment BorderSharingValidator::make_segment(
  const Point & a, const Point & b)
{
  return a < b ? Segment{a, b} : Segment{b, a};
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
