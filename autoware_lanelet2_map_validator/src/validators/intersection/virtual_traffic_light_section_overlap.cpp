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

#include "lanelet2_map_validator/validators/intersection/virtual_traffic_light_section_overlap.hpp"

#include "lanelet2_core/primitives/Traits.h"
#include "lanelet2_map_validator/utils.hpp"

#include <boost/geometry.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/LaneletMap.h>
#include <lanelet2_core/primitives/LineString.h>
#include <lanelet2_routing/RoutingGraph.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<VirtualTrafficLightSectionOverlapValidator> reg;
}

lanelet::validation::Issues VirtualTrafficLightSectionOverlapValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(
    issues, check_virtual_traffic_light_section_overlap(map));

  return issues;
}

lanelet::validation::Issues
VirtualTrafficLightSectionOverlapValidator::check_virtual_traffic_light_section_overlap(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  traffic_rules_ = lanelet::traffic_rules::TrafficRulesFactory::create(
    lanelet::Locations::Germany, lanelet::Participants::Vehicle);
  routing_graph_ptr_ = lanelet::routing::RoutingGraph::build(map, *traffic_rules_);

  std::map<std::string, std::string> reg_elem_id_map;

  for (const auto & lane : map.laneletLayer) {
    // Check the lanelet has a virtual_traffic_light regulatory element
    // and get the path ending with that lanelet
    const auto reg_elems = lane.regulatoryElementsAs<VirtualTrafficLight>();
    if (reg_elems.empty()) {
      continue;
    }
    const auto reg_elem = reg_elems.front();
    const auto all_paths = get_all_possible_paths(reg_elem, map);
    auto it = std::find_if(
      all_paths.begin(), all_paths.end(), [lane](const lanelet::routing::LaneletPath & v) {
        return !v.empty() && v.back().id() == lane.id();
      });

    if (it == all_paths.end()) {
      continue;
    }
    const auto target_path = *it;
    const auto bbox = get_lanelet_path_bbox(target_path);
    const auto nearby_lanelets = map.laneletLayer.search(bbox);

    // Collect nearby virtual traffic lights
    std::vector<VirtualTrafficLight::ConstPtr> nearby_reg_elems;
    for (const auto & nearby_lane : nearby_lanelets) {
      const auto temp_reg_elems = nearby_lane.regulatoryElementsAs<VirtualTrafficLight>();
      if (temp_reg_elems.empty()) {
        continue;
      }
      const auto temp_reg_elem = temp_reg_elems.front();
      if (
        temp_reg_elem->id() == reg_elem->id() || !is_target_virtual_traffic_light(temp_reg_elem)) {
        continue;
      }
      bool is_already_counted = std::any_of(
        nearby_reg_elems.begin(), nearby_reg_elems.end(),
        [temp_reg_elem](const VirtualTrafficLight::ConstPtr & vtl) {
          return vtl->id() == temp_reg_elem->id();
        });
      if (!is_already_counted) {
        nearby_reg_elems.push_back(temp_reg_elem);
      }
    }

    // Check whether nearby virtual traffic lights have overlapping paths
    for (const auto & nearby_reg_elem : nearby_reg_elems) {
      const auto all_nearby_paths = get_all_possible_paths(nearby_reg_elem, map);
      for (const auto & nearby_path : all_nearby_paths) {
        const auto overlaps = get_overlapped_lanelets(target_path, nearby_path);
        if (overlaps.empty()) {
          continue;
        }

        if (overlaps.size() > 1) {
          reg_elem_id_map["reg_elem_id"] = std::to_string(nearby_reg_elem->id());
          issues.emplace_back(construct_issue_from_code(
            issue_code(this->name(), 1), reg_elem->id(), reg_elem_id_map));
          continue;
        }

        const auto left_bound = overlaps.front().leftBound();
        const auto right_bound = overlaps.front().rightBound();

        lanelet::ConstLineString3d end_line, start_line;
        if (target_path.back() == overlaps.front()) {
          end_line =
            select_end_line(
              reg_elem->getParameters<lanelet::ConstLineString3d>("end_line"), overlaps.front())
              .get();
          start_line =
            nearby_reg_elem->getParameters<lanelet::ConstLineString3d>("start_line").front();
        } else {
          end_line = select_end_line(
                       nearby_reg_elem->getParameters<lanelet::ConstLineString3d>("end_line"),
                       overlaps.front())
                       .get();
          start_line = reg_elem->getParameters<lanelet::ConstLineString3d>("start_line").front();
        }
        const auto end_aligned =
          get_aligned_linestring(end_line, lanelet::CompoundLineString3d({left_bound}));
        const auto start_aligned =
          get_aligned_linestring(start_line, lanelet::CompoundLineString3d({left_bound}));

        const auto left_bound_basic_2d = lanelet::traits::to2D(left_bound.basicLineString());
        const auto right_bound_basic_2d = lanelet::traits::to2D(right_bound.basicLineString());
        const auto end_left_arc = lanelet::geometry::toArcCoordinates(
          left_bound_basic_2d, end_aligned.front().basicPoint2d());
        const auto end_right_arc = lanelet::geometry::toArcCoordinates(
          right_bound_basic_2d, end_aligned.back().basicPoint2d());

        const auto start_left_arc = lanelet::geometry::toArcCoordinates(
          left_bound_basic_2d, start_aligned.front().basicPoint2d());
        const auto start_right_arc = lanelet::geometry::toArcCoordinates(
          right_bound_basic_2d, start_aligned.back().basicPoint2d());

        if (
          end_left_arc.length > start_left_arc.length ||
          end_right_arc.length > start_right_arc.length) {
          reg_elem_id_map["reg_elem_id"] = std::to_string(nearby_reg_elem->id());
          issues.emplace_back(construct_issue_from_code(
            issue_code(this->name(), 1), reg_elem->id(), reg_elem_id_map));
        }
      }
    }
  }
  return issues;
}

bool VirtualTrafficLightSectionOverlapValidator::is_target_virtual_traffic_light(
  const lanelet::RegulatoryElementConstPtr & reg_elem)
{
  bool is_virtual_traffic_light =
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

lanelet::routing::LaneletPaths VirtualTrafficLightSectionOverlapValidator::get_all_possible_paths(
  const lanelet::RegulatoryElementConstPtr & reg_elem, const lanelet::LaneletMap & map)
{
  lanelet::routing::LaneletPaths all_paths;

  // get starting lanelet
  const lanelet::ConstLineString3d start_line =
    reg_elem->getParameters<lanelet::ConstLineString3d>("start_line").front();
  const lanelet::ConstLanelet start_lanelet = belonging_lanelet(start_line, map).get();

  // get all referrer lanelets (= end lanelets)
  const lanelet::ConstLanelets end_lanelets = map.laneletLayer.findUsages(reg_elem);

  for (const auto & end_lanelet : end_lanelets) {
    const auto lanelet_path_opt =
      routing_graph_ptr_->shortestPath(start_lanelet, end_lanelet, {}, false);

    if (!!lanelet_path_opt) {
      all_paths.push_back(lanelet_path_opt.get());
    }
  }
  return all_paths;
}

lanelet::BoundingBox2d VirtualTrafficLightSectionOverlapValidator::get_lanelet_path_bbox(
  const lanelet::routing::LaneletPath & path)
{
  lanelet::BoundingBox2d bbox;
  for (const ConstLanelet & lane : path) {
    bbox.extend(lanelet::geometry::boundingBox2d(lane));
  }
  return bbox;
}

lanelet::ConstLanelets VirtualTrafficLightSectionOverlapValidator::get_overlapped_lanelets(
  const lanelet::routing::LaneletPath & path1, const lanelet::routing::LaneletPath & path2)
{
  // Note that this function only return values if the overlap is happening at the end of both paths
  // or one is a subsequence of the other.

  // Check path1 is a full subsequence of path2
  if (std::search(path2.begin(), path2.end(), path1.begin(), path1.end()) != path2.end()) {
    return lanelet::ConstLanelets(path1.begin(), path1.end());
  }

  // Check path2 is a full subsequence of path1
  if (std::search(path1.begin(), path1.end(), path2.begin(), path2.end()) != path1.end()) {
    return lanelet::ConstLanelets(path2.begin(), path2.end());
  }

  int32_t max_overlap = std::min(path1.size(), path2.size());

  // Compare the end of path1 and the start of path2
  for (int32_t len = max_overlap; len > 0; --len) {
    if (std::equal(path1.end() - len, path1.end(), path2.begin(), path2.begin() + len)) {
      return lanelet::ConstLanelets(path1.end() - len, path1.end());
    }
  }

  // Compare the end of path2 and the start of path1
  for (int32_t len = max_overlap; len > 0; --len) {
    if (std::equal(path2.end() - len, path2.end(), path1.begin(), path1.begin() + len)) {
      return lanelet::ConstLanelets(path2.end() - len, path2.end());
    }
  }

  return lanelet::ConstLanelets();
}

}  // namespace lanelet::autoware::validation
