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
    if (
      reg_elem->attributeOr(lanelet::AttributeName::Subtype, std::string("")) !=
      VirtualTrafficLight::RuleName) {
      continue;
    }

    // get pairs of end_line and referrer lanelets
    std::vector<std::pair<lanelet::Optional<lanelet::ConstLineString3d>, lanelet::ConstLanelet>>
      end_pairs;
    const ConstLineStrings3d end_lines =
      reg_elem->getParameters<lanelet::ConstLineString3d>("end_line");
    const ConstLanelets referrer_lanelets = map.laneletLayer.findUsages(reg_elem);
    for (const auto & lane : referrer_lanelets) {
      end_pairs.push_back({select_end_line(end_lines, lane), lane});
    }

    for (const auto & [end_line_opt, referrer_lanelet] : end_pairs) {
      const lanelet::ConstLineString3d start_line =
        reg_elem->getParameters<lanelet::ConstLineString3d>("start_line").front();
      const lanelet::ConstLineString3d stop_line =
        reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine).front();

      if (!end_line_opt) {
        const std::string issue_message = "Cannot find the end_line for referrer lanelet " +
                                          std::to_string(referrer_lanelet.id()) +
                                          ". The end_line should cover the referrer lanelet.";
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement,
          reg_elem->id(), append_issue_code_prefix(this->name(), 1, issue_message));
        continue;
      }
      const lanelet::ConstLineString3d end_line = end_line_opt.get();

      const lanelet::Optional<lanelet::ConstLanelet> start_lanelet =
        belonging_lanelet(start_line, map);
      if (!start_lanelet) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
          start_line.id(),
          append_issue_code_prefix(this->name(), 2, "The start_line isn't placed over a lanelet."));
        continue;
      }
      if (boost::geometry::intersects(
            lanelet::traits::to2D(start_line.basicLineString()),
            referrer_lanelet.polygon2d().basicPolygon())) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
          start_line.id(),
          append_issue_code_prefix(
            this->name(), 3,
            "The start_line must not intersect with the referrer lanelet of the virtual traffic "
            "light."));
        continue;
      }

      const lanelet::Optional<lanelet::ConstLanelet> stop_lanelet =
        belonging_lanelet(stop_line, map, referrer_lanelets);
      if (!stop_lanelet) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
          stop_line.id(),
          append_issue_code_prefix(this->name(), 4, "The stop_line isn't placed over a lanelet."));
        continue;
      }

      const lanelet::ConstLineString3d start_line_aligned =
        get_aligned_linestring(start_line, start_lanelet.get().leftBound());
      const lanelet::ConstLineString3d stop_line_aligned =
        get_aligned_linestring(stop_line, stop_lanelet.get().leftBound());
      const lanelet::ConstLineString3d end_line_aligned =
        get_aligned_linestring(end_line, referrer_lanelet.leftBound());

      lanelet::Optional<lanelet::routing::LaneletPath> start_to_stop_path_opt =
        routing_graph_ptr->shortestPath(start_lanelet.get(), stop_lanelet.get(), {}, false);
      if (!start_to_stop_path_opt) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement,
          reg_elem->id(),
          append_issue_code_prefix(
            this->name(), 5, "Cannot find a lanelet path from start_line to end_line."));
        continue;
      }
      std::cout << "start_to_stop" << std::endl;
      for (const auto & ll : start_to_stop_path_opt.get()) {
        std::cout << ll.id() << std::endl;
      }
      lanelet::Optional<lanelet::routing::LaneletPath> stop_to_end_path_opt =
        routing_graph_ptr->shortestPath(stop_lanelet.get(), referrer_lanelet, {}, false);
      std::cout << "stop_to_end" << std::endl;
      if (!stop_to_end_path_opt) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement,
          reg_elem->id(),
          append_issue_code_prefix(
            this->name(), 5, "Cannot find a lanelet path from start_line to end_line."));
        continue;
      }
      for (const auto & ll : stop_to_end_path_opt.get()) {
        std::cout << ll.id() << std::endl;
      }

      lanelet::routing::LaneletPath shortest_path =
        routing_graph_ptr->shortestPath(start_lanelet.get(), referrer_lanelet, {}, false).get();
      const auto concat_left_bound = get_concatenated_bound(shortest_path, true);
      const auto concat_right_bound = get_concatenated_bound(shortest_path, false);

      bool is_ok = is_ordered_in_length_manner(
                     start_line_aligned.front(), stop_line_aligned.front(),
                     end_line_aligned.front(), concat_left_bound) &&
                   is_ordered_in_length_manner(
                     start_line_aligned.back(), stop_line_aligned.back(), end_line_aligned.back(),
                     concat_right_bound);
      if (!is_ok) {
        const std::string issue_message =
          "Then order of start line, stop line, and end line is wrong (end_line ID: " +
          std::to_string(end_line.id()) + ")";
        issues.emplace_back(
          lanelet::validation::Severity::Warning, lanelet::validation::Primitive::RegulatoryElement,
          reg_elem->id(), append_issue_code_prefix(this->name(), 6, issue_message));
        continue;
      }
    }
  }

  return issues;
}

lanelet::Optional<lanelet::ConstLanelet> VirtualTrafficLightLineOrderValidator::belonging_lanelet(
  const lanelet::ConstLineString3d & linestring, const lanelet::LaneletMap & map,
  const lanelet::ConstLanelets & excepts)
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
    if (std::find(excepts.begin(), excepts.end(), lane) != excepts.end()) {
      continue;
    }
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
  const lanelet::ConstLineString3d & linestring, const lanelet::ConstLanelet & lane)
{
  lanelet::BasicLineString2d linestring2d = lanelet::traits::to2D(linestring.basicLineString());
  std::vector<lanelet::BasicLineString2d> intersections;

  boost::geometry::intersection(linestring2d, lane.polygon2d().basicPolygon(), intersections);
  if (intersections.empty()) {
    return 0.0;
  }

  double covered_length = 0.0;
  for (const auto & segment : intersections) {
    covered_length += boost::geometry::length(segment);
  }

  return covered_length / boost::geometry::length(linestring2d);
}

lanelet::ConstLineString3d VirtualTrafficLightLineOrderValidator::get_aligned_linestring(
  const lanelet::ConstLineString3d & linestring, const lanelet::ConstLineString3d & base_arc)
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

  return p1_arc.length <= p2_arc.length && p2_arc.length <= p3_arc.length;
}
}  // namespace lanelet::autoware::validation
