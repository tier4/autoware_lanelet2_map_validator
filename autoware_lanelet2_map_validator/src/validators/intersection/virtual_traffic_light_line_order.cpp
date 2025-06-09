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

#include "lanelet2_map_validator/validators/intersection/virtual_traffic_light_line_order.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

#include <boost/geometry.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/LaneletMap.h>
#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<VirtualTrafficLightLineOrderValidator> reg;
}

lanelet::validation::Issues VirtualTrafficLightLineOrderValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_virtual_traffic_light_line_order(map));

  return issues;
}

lanelet::validation::Issues
VirtualTrafficLightLineOrderValidator::check_virtual_traffic_light_line_order(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::traffic_rules::TrafficRulesPtr traffic_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      lanelet::Locations::Germany, lanelet::Participants::Vehicle);
  lanelet::routing::RoutingGraphUPtr routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  for (const auto & reg_elem : map.regulatoryElementLayer) {
    if (!is_target_virtual_traffic_light(reg_elem)) {
      continue;
    }

    // retrieve start_line and stop_line with their corresponding lanelet.
    const lanelet::ConstLineString3d start_line =
      reg_elem->getParameters<lanelet::ConstLineString3d>("start_line").front();
    const lanelet::ConstLineString3d stop_line =
      reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine).front();
    const lanelet::Optional<lanelet::ConstLanelet> start_lanelet =
      belonging_lanelet(start_line, map);
    const lanelet::Optional<lanelet::ConstLanelet> stop_lanelet = belonging_lanelet(stop_line, map);

    if (!start_lanelet) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), start_line.id()));
      continue;
    }

    // get pairs of end_line and referrer lanelets and check start_line not touching the referrer
    // meanwhile
    std::vector<std::pair<lanelet::ConstLineString3d, lanelet::ConstLanelet>> end_pairs;
    const lanelet::ConstLineStrings3d end_lines =
      reg_elem->getParameters<lanelet::ConstLineString3d>("end_line");
    const lanelet::ConstLanelets referrer_lanelets = map.laneletLayer.findUsages(reg_elem);
    bool is_start_line_intersecting = false;
    for (const auto & lane : referrer_lanelets) {
      const auto end_line_opt = select_end_line(end_lines, lane);
      if (!end_line_opt) {
        std::map<std::string, std::string> lane_id_map;
        lane_id_map["id"] = std::to_string(lane.id());
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 2), reg_elem->id(), lane_id_map));
        continue;
      }
      end_pairs.push_back({end_line_opt.get(), lane});

      if (boost::geometry::intersects(
            lanelet::traits::to2D(start_line.basicLineString()), lane.polygon2d().basicPolygon())) {
        is_start_line_intersecting = true;
      }
    }

    if (is_start_line_intersecting) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 3), start_line.id()));
      continue;
    }

    // Validate line orders for each end_line
    for (const auto & [end_line, referrer_lanelet] : end_pairs) {
      std::map<std::string, std::string> end_line_id_map;
      end_line_id_map["id"] = std::to_string(end_line.id());
      lanelet::Optional<lanelet::routing::LaneletPath> start_to_end_path_opt =
        routing_graph_ptr->shortestPath(start_lanelet.get(), referrer_lanelet, {}, false);
      if (!start_to_end_path_opt) {
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 4), reg_elem->id(), end_line_id_map));
        continue;
      }

      const auto concat_left_bound = get_concatenated_bound(start_to_end_path_opt.get(), true);
      const auto concat_right_bound = get_concatenated_bound(start_to_end_path_opt.get(), false);
      const lanelet::CompoundPolygon3d concat_lanelet_polygon(
        {concat_left_bound, concat_right_bound.invert()});

      if (intersection_ratio(stop_line, concat_lanelet_polygon) < 0.1) {
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 5), stop_line.id(), end_line_id_map));
      }

      const lanelet::ConstLineString3d start_line_aligned =
        get_aligned_linestring(start_line, concat_left_bound);
      const lanelet::ConstLineString3d stop_line_aligned =
        get_aligned_linestring(stop_line, concat_left_bound);
      const lanelet::ConstLineString3d end_line_aligned =
        get_aligned_linestring(end_line, concat_left_bound);

      bool is_ok = is_ordered_in_length_manner(
                     start_line_aligned.front(), stop_line_aligned.front(),
                     end_line_aligned.front(), concat_left_bound) &&
                   is_ordered_in_length_manner(
                     start_line_aligned.back(), stop_line_aligned.back(), end_line_aligned.back(),
                     concat_right_bound);
      if (!is_ok) {
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 6), reg_elem->id(), end_line_id_map));
        continue;
      }
    }
  }

  return issues;
}

bool VirtualTrafficLightLineOrderValidator::is_target_virtual_traffic_light(
  const lanelet::RegulatoryElementConstPtr & reg_elem)
{
  const bool is_virtual_traffic_light =
    reg_elem->attributeOr(lanelet::AttributeName::Subtype, std::string("")) ==
    VirtualTrafficLight::RuleName;

  if (!is_virtual_traffic_light) {
    return false;
  }

  // if refers type is not specifed all virtual traffic lights are targets
  if (target_refers_.empty()) {
    return true;
  }

  const auto refers =
    reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers).front();
  bool is_target_refers =
    std::find(
      target_refers_.begin(), target_refers_.end(),
      refers.attributeOr(lanelet::AttributeName::Type, std::string(""))) != target_refers_.end();

  return is_target_refers;
}

lanelet::Optional<lanelet::ConstLanelet> VirtualTrafficLightLineOrderValidator::belonging_lanelet(
  const lanelet::ConstLineString3d & linestring, const lanelet::LaneletMap & map)
{
  lanelet::BasicPoint3d centerpoint =
    (linestring.front().basicPoint() + linestring.back().basicPoint()) / 2;
  auto candidate_lanelets = lanelet::geometry::findWithin3d(map.laneletLayer, centerpoint, 1.0);
  if (candidate_lanelets.empty()) {
    return {};
  }

  double max_ratio = -1.0;
  lanelet::ConstLanelet result;
  for (const auto & [_, lane] : candidate_lanelets) {
    const double ratio = intersection_ratio(linestring, lane);
    if (ratio > max_ratio) {
      max_ratio = ratio;
      result = lane;
    }
  }

  if (max_ratio <= 0.0) {
    return {};
  }
  return result;
}

lanelet::Optional<lanelet::ConstLineString3d>
VirtualTrafficLightLineOrderValidator::select_end_line(
  const lanelet::ConstLineStrings3d & candidate_lines, const lanelet::ConstLanelet & base_lanelet)
{
  double max_ratio = -1.0;
  lanelet::ConstLineString3d result;
  for (const auto & candidate : candidate_lines) {
    const double ratio = intersection_ratio(candidate, base_lanelet);
    if (ratio > max_ratio) {
      max_ratio = ratio;
      result = candidate;
    }
  }

  if (max_ratio <= 0.0) {
    return {};
  }
  return result;
}

double VirtualTrafficLightLineOrderValidator::intersection_ratio(
  const lanelet::ConstLineString3d & linestring, const lanelet::CompoundPolygon3d & polygon)
{
  lanelet::BasicLineString2d linestring2d = lanelet::traits::to2D(linestring.basicLineString());
  std::vector<lanelet::BasicLineString2d> intersections;

  boost::geometry::intersection(
    linestring2d, lanelet::traits::to2D(polygon).basicPolygon(), intersections);
  if (intersections.empty()) {
    return 0.0;
  }

  double covered_length = 0.0;
  for (const auto & segment : intersections) {
    covered_length += boost::geometry::length(segment);
  }
  return covered_length / boost::geometry::length(linestring2d);
}

double VirtualTrafficLightLineOrderValidator::intersection_ratio(
  const lanelet::ConstLineString3d & linestring, const lanelet::ConstLanelet & lane)
{
  return intersection_ratio(linestring, lane.polygon3d());
}

lanelet::ConstLineString3d VirtualTrafficLightLineOrderValidator::get_aligned_linestring(
  const lanelet::ConstLineString3d & linestring, const lanelet::CompoundLineString3d & base_arc)
{
  // Align in the sense that linestrings are drawn from left to right as viewed from the vehicle
  lanelet::ArcCoordinates arc_front = lanelet::geometry::toArcCoordinates(
    lanelet::traits::to2D(base_arc), linestring.front().basicPoint2d());
  lanelet::ArcCoordinates arc_back = lanelet::geometry::toArcCoordinates(
    lanelet::traits::to2D(base_arc), linestring.back().basicPoint2d());

  return (arc_front.distance > arc_back.distance) ? linestring : linestring.invert();
}

lanelet::CompoundLineString3d VirtualTrafficLightLineOrderValidator::get_concatenated_bound(
  const lanelet::routing::LaneletPath & path, const bool get_left)
{
  lanelet::ConstLineStrings3d bounds;

  for (size_t i = 0; i < path.size(); i++) {
    const lanelet::ConstLineString3d line = (get_left) ? path[i].leftBound() : path[i].rightBound();
    if (line.empty()) {
      continue;
    }
    bounds.push_back(line);
  }

  return lanelet::CompoundLineString3d(bounds);
}

bool VirtualTrafficLightLineOrderValidator::is_ordered_in_length_manner(
  const lanelet::ConstPoint3d & p1, const lanelet::ConstPoint3d & p2,
  const lanelet::ConstPoint3d & p3, const lanelet::CompoundLineString3d & base_arc)
{
  const auto p1_arc =
    lanelet::geometry::toArcCoordinates(lanelet::traits::to2D(base_arc), p1.basicPoint2d());
  const auto p2_arc =
    lanelet::geometry::toArcCoordinates(lanelet::traits::to2D(base_arc), p2.basicPoint2d());
  const auto p3_arc =
    lanelet::geometry::toArcCoordinates(lanelet::traits::to2D(base_arc), p3.basicPoint2d());

  return p1_arc.length < p2_arc.length && p2_arc.length < p3_arc.length;
}
}  // namespace lanelet::autoware::validation
