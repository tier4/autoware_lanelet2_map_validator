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

#include "lanelet2_map_validator/validators/intersection/roundabout_configuration.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/BasicRegulatoryElements.h>
#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RoundaboutConfigurationValidator> reg;
}

lanelet::validation::Issues RoundaboutConfigurationValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_roundabout_configuration(map));

  return issues;
}

lanelet::validation::Issues RoundaboutConfigurationValidator::check_roundabout_configuration(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  std::vector<lanelet::RegulatoryElementConstPtr> roundabout_reg_elems;
  
  for (const auto & reg_elem : map.regulatoryElementLayer) {
    const auto & attrs = reg_elem->attributes();
    const auto & subtype_it = attrs.find(lanelet::AttributeName::Subtype);
    
    if (subtype_it != attrs.end() && subtype_it->second == "roundabout") {
      roundabout_reg_elems.push_back(reg_elem);
    }
  }

  auto traffic_rules = lanelet::traffic_rules::TrafficRulesFactory::create(
    lanelet::Locations::Germany, lanelet::Participants::Vehicle);
  auto routing_graph = lanelet::routing::RoutingGraph::build(map, *traffic_rules);

  for (const auto & roundabout_elem : roundabout_reg_elems) {
    auto entry_lanelets = roundabout_elem->getParameters<lanelet::ConstLanelet>("entry");
    auto exit_lanelets = roundabout_elem->getParameters<lanelet::ConstLanelet>("exit");
    auto internal_lanelets = roundabout_elem->getParameters<lanelet::ConstLanelet>("internal");

    // Check that each internal lanelet shares endpoints (by point ID) with entry or exit lanelets
    for (const auto & internal_ll : internal_lanelets) {
      const auto & internal_centerline = internal_ll.centerline();
      if (internal_centerline.size() < 2) {
        continue; // Skip invalid lanelets
      }
      
      auto internal_start_id = internal_centerline.front().id();
      auto internal_end_id = internal_centerline.back().id();

      bool has_entry_connection = false;
      bool has_exit_connection = false;

      for (const auto & entry_ll : entry_lanelets) {
        const auto & entry_centerline = entry_ll.centerline();
        if (entry_centerline.size() < 2) continue;
        
        auto entry_end_id = entry_centerline.back().id();
        
        if (internal_start_id == entry_end_id || internal_end_id == entry_end_id) {
          has_entry_connection = true;
          break;
        }
      }

      for (const auto & exit_ll : exit_lanelets) {
        const auto & exit_centerline = exit_ll.centerline();
        if (exit_centerline.size() < 2) continue;
        
        auto exit_start_id = exit_centerline.front().id();
        
        if (internal_start_id == exit_start_id || internal_end_id == exit_start_id) {
          has_exit_connection = true;
          break;
        }
      }

      if (!has_entry_connection || !has_exit_connection) {
        std::map<std::string, std::string> substitution_map;
        substitution_map["internal_lanelet_id"] = std::to_string(internal_ll.id());
        issues.emplace_back(construct_issue_from_code(
          issue_code(this->name(), 1), roundabout_elem->id(), substitution_map));
      }
    }

    std::set<lanelet::Id> entry_ids, exit_ids, internal_ids;
    for (const auto & ll : entry_lanelets) entry_ids.insert(ll.id());
    for (const auto & ll : exit_lanelets) exit_ids.insert(ll.id());
    for (const auto & ll : internal_lanelets) internal_ids.insert(ll.id());

    for (const auto & entry_ll : entry_lanelets) {
      for (const auto & exit_ll : exit_lanelets) {
        lanelet::Optional<lanelet::routing::LaneletPath> path_opt =
          routing_graph->shortestPath(entry_ll, exit_ll, {}, false);
        
        if (!path_opt) {
          continue;
        }

        const auto & path = path_opt.get();
        for (const auto & lanelet_in_path : path) {
          bool is_valid = 
            entry_ids.count(lanelet_in_path.id()) > 0 ||
            exit_ids.count(lanelet_in_path.id()) > 0 ||
            internal_ids.count(lanelet_in_path.id()) > 0;

          if (!is_valid) {
            // Issue-002: Route contains lanelet that is not entry, exit, or internal
            std::map<std::string, std::string> substitution_map;
            substitution_map["entry_lanelet_id"] = std::to_string(entry_ll.id());
            substitution_map["exit_lanelet_id"] = std::to_string(exit_ll.id());
            substitution_map["invalid_lanelet_id"] = std::to_string(lanelet_in_path.id());
            issues.emplace_back(construct_issue_from_code(
              issue_code(this->name(), 2), roundabout_elem->id(), substitution_map));
            break;
          }
        }
      }
    }

    for (const auto & internal_ll : internal_lanelets) {
      const auto conflicting_primitives = routing_graph->conflicting(internal_ll);
      
      bool has_conflicting_lanelet = false;
      for (const auto & primitive : conflicting_primitives) {
        if (!primitive.isArea()) {
          auto internal_polygon = internal_ll.polygon2d().basicPolygon();
          auto conflicting_polygon = primitive.lanelet()->polygon2d().basicPolygon();
          double coverage_ratio = polygon_overlap_ratio(internal_polygon, conflicting_polygon);
          
          if (coverage_ratio >= 0.01) {
            has_conflicting_lanelet = true;
            break;
          }
        }
      }

      if (!has_conflicting_lanelet) {
        if (!internal_ll.hasAttribute("enable_exit_turn_signal")) {
          // Issue-003: Internal lanelet without conflicts missing enable_exit_turn_signal tag
          std::map<std::string, std::string> substitution_map;
          substitution_map["internal_lanelet_id"] = std::to_string(internal_ll.id());
          issues.emplace_back(construct_issue_from_code(
            issue_code(this->name(), 3), roundabout_elem->id(), substitution_map));
        } else {
          std::string tag_value = internal_ll.attribute("enable_exit_turn_signal").value();
          if (tag_value != "true") {
            // Issue-004: Internal lanelet without conflicts has enable_exit_turn_signal set to non-true value
            std::map<std::string, std::string> substitution_map;
            substitution_map["internal_lanelet_id"] = std::to_string(internal_ll.id());
            substitution_map["tag_value"] = tag_value;
            issues.emplace_back(construct_issue_from_code(
              issue_code(this->name(), 4), roundabout_elem->id(), substitution_map));
          }
        }
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
