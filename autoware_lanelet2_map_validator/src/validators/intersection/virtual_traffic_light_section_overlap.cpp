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
#include <string>
#include <unordered_set>

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

  for (const auto & reg_elem : map.regulatoryElementLayer) {
    if (!is_target_virtual_traffic_light(reg_elem)) {
      continue;
    }

    // get all lanelet paths
    const auto all_paths = get_all_possible_paths(reg_elem, map);

    for (const auto & path : all_paths) {
      // find other virtual traffic lights nearby
      const auto bbox = get_lanelet_path_bbox(all_paths.front());
      const auto nearby_lanelets = map.laneletLayer.search(bbox);
      for (const auto & lane : nearby_lanelets) {
        const auto nearby_reg_elems = lane.regulatoryElementsAs<VirtualTrafficLight>();
        if (
          nearby_reg_elems.empty() || !is_target_virtual_traffic_light(nearby_reg_elems.front())) {
          continue;
        }
        const auto all_nearby_paths = get_all_possible_paths(nearby_reg_elems.front(), map);
        for (const auto & nearby_path : all_nearby_paths) {
          const auto overlaps = get_overlapped_lanelets(path, nearby_path);
          if (overlaps.empty()) {
            continue;
          }

          if (overlaps.size() > 1) {
            issues.emplace_back(
              construct_issue_from_code(issue_code(this->name(), 1), reg_elem->id()));
          }

          const auto left_bound = overlaps.front().leftBound2d();
          const auto right_bound = overlaps.front().leftBound2d();
        }
      }
    }
  }

  (void)map;

  return issues;
}

bool VirtualTrafficLightSectionOverlapValidator::is_target_virtual_traffic_light(
  const lanelet::RegulatoryElementConstPtr & reg_elem)
{
  bool is_virtual_traffic_light =
    reg_elem->attributeOr(lanelet::AttributeName::Subtype, std::string("")) ==
    VirtualTrafficLight::RuleName;

  const auto refers =
    reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers).front();
  bool is_target_refers =
    std::find(
      target_refers_.begin(), target_refers_.end(),
      refers.front().attributeOr(lanelet::AttributeName::Type, std::string(""))) !=
    target_refers_.end();

  return is_virtual_traffic_light & is_target_refers;
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

lanelet::Ids VirtualTrafficLightSectionOverlapValidator::common_lanelet_ids(
  const lanelet::routing::LaneletPath & path1, const lanelet::routing::LaneletPath & path2)
{
  std::unordered_set<lanelet::Id> id_set1;
  lanelet::Ids common_ids;
  for (const auto & lane1 : path1) {
    id_set1.insert(lane1.id());
  }

  for (const auto & lane2 : path2) {
    if (id_set1.count(lane2.id())) {
      common_ids.push_back(lane2.id());
    }
  }
  return common_ids;
}

lanelet::ConstLanelets VirtualTrafficLightSectionOverlapValidator::get_overlapped_lanelets(
  const lanelet::routing::LaneletPath & path1, const lanelet::routing::LaneletPath & path2)
{
  // Note that this function only return values if the overlap is happening at the end of both paths
  // or one is a subsequence of the other.

  // Check path1 is a full subsequence of path2
  if (std::search(path2.begin(), path2.end(), path1.begin(), path1.begin()) != path2.end()) {
    return lanelet::ConstLanelets(path1.begin(), path1.end());
  }

  // Check path2 is a full subsequence of path1
  if (std::search(path1.begin(), path1.end(), path2.begin(), path2.begin()) != path1.end()) {
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
