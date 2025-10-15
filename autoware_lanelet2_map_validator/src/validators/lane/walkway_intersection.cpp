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

#include "lanelet2_map_validator/validators/lane/walkway_intersection.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/intersection.hpp>

#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_core/geometry/Polygon.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<WalkwayIntersectionValidator> reg;
}

lanelet::validation::Issues WalkwayIntersectionValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_walkway_intersection(map));

  return issues;
}

lanelet::validation::Issues WalkwayIntersectionValidator::check_walkway_intersection(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  // Create routing graph for checking lanelet relationships
  lanelet::traffic_rules::TrafficRulesPtr traffic_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      "validator", lanelet::Participants::Vehicle);

  lanelet::routing::RoutingGraphUPtr routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  std::vector<lanelet::ConstLanelet> walkway_lanelets;
  std::vector<lanelet::ConstLanelet> road_lanelets;

  for (const auto & lanelet : map.laneletLayer) {
    if (!lanelet.hasAttribute(lanelet::AttributeName::Subtype)) {
      continue;
    }

    const auto subtype = lanelet.attribute(lanelet::AttributeName::Subtype).value();
    if (subtype == "walkway") {
      walkway_lanelets.push_back(lanelet);
    } else if (subtype == lanelet::AttributeValueString::Road) {
      road_lanelets.push_back(lanelet);
    }
  }

  for (const auto & walkway : walkway_lanelets) {
    const auto walkway_polygon = walkway.polygon2d().basicPolygon();

    // Issue-001: Check if walkway has conflicting relationship with any road lanelet
    // Issue-002: Check if walkway extends at least 3 meters from intersection
    const auto conflicting_lanelets = routing_graph_ptr->conflicting(walkway);
    bool has_road_conflict = false;

    for (const auto & road : road_lanelets) {
      const auto road_polygon = road.polygon2d().basicPolygon();

      // Check if this specific road is in the conflicting list
      auto it = std::find_if(
        conflicting_lanelets.begin(), conflicting_lanelets.end(),
        [&road](const auto & conflicting) { return conflicting.id() == road.id(); });

      if (it != conflicting_lanelets.end()) {
        has_road_conflict = true;
        std::vector<lanelet::BasicPolygon2d> intersection_polygons;
        boost::geometry::intersection(walkway_polygon, road_polygon, intersection_polygons);

        if (!intersection_polygons.empty()) {
          const auto & intersection_polygon = intersection_polygons[0];

          const lanelet::BasicLineString2d starting_edge = {
            walkway.leftBound2d().front().basicPoint2d(),
            walkway.rightBound2d().front().basicPoint2d()};
          const lanelet::BasicLineString2d ending_edge = {
            walkway.leftBound2d().back().basicPoint2d(),
            walkway.rightBound2d().back().basicPoint2d()};

          const double start_extension =
            boost::geometry::distance(intersection_polygon, starting_edge);
          const double end_extension = boost::geometry::distance(intersection_polygon, ending_edge);

          const double min_extension_threshold = 3.0;  // 3 meters
          const double actual_extension = std::min(start_extension, end_extension);

          if (actual_extension < min_extension_threshold) {
            std::map<std::string, std::string> substitutions_ext;
            substitutions_ext["road_lanelet_id"] = std::to_string(road.id());
            substitutions_ext["extension"] = std::to_string(actual_extension);
            issues.push_back(construct_issue_from_code(
              issue_code(this->name(), 2), walkway.id(), substitutions_ext));
          }
        }
      }
    }

    // Report Issue-001 if no road conflicts were found
    if (!has_road_conflict) {
      std::map<std::string, std::string> substitutions;
      issues.push_back(
        construct_issue_from_code(issue_code(this->name(), 1), walkway.id(), substitutions));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
