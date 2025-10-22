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

#include "lanelet2_map_validator/validators/area/missing_regulatory_elements_for_bus_stop_areas.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/bus_stop_area.hpp>
#include <range/v3/view/filter.hpp>

#include <lanelet2_core/LaneletMap.h>

#include <algorithm>
#include <set>
#include <vector>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<MissingRegulatoryElementsForBusStopAreas> reg;
}

lanelet::validation::Issues MissingRegulatoryElementsForBusStopAreas::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(
    issues, check_missing_regulatory_elements_for_bus_stop_areas(map));

  return issues;
}

lanelet::validation::Issues
MissingRegulatoryElementsForBusStopAreas::check_missing_regulatory_elements_for_bus_stop_areas(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  std::set<lanelet::Id> bus_stop_area_polygon_ids;

  for (const auto & polygon : map.polygonLayer) {
    if (
      polygon.hasAttribute(lanelet::AttributeName::Type) &&
      polygon.attribute(lanelet::AttributeName::Type) == lanelet::autoware::BusStopArea::RuleName) {
      bus_stop_area_polygon_ids.insert(polygon.id());
    }
  }

  auto reg_elem_with_bus_stop_polygon =
    map.regulatoryElementLayer | ranges::views::filter([](auto && elem) {
      const auto & attrs = elem->attributes();
      const auto & it = attrs.find(lanelet::AttributeName::Subtype);
      return it != attrs.end() && it->second == lanelet::autoware::BusStopArea::RuleName;
    }) |
    ranges::views::filter([](auto && elem) {
      const auto & param = elem->getParameters();
      return param.find(lanelet::RoleNameString::Refers) != param.end();
    });

  std::set<lanelet::Id> bus_stop_area_polygon_ids_reg_elem;
  for (const auto & elem : reg_elem_with_bus_stop_polygon) {
    const auto & refers = elem->getParameters<lanelet::ConstPolygon3d>(lanelet::RoleName::Refers);
    for (const lanelet::ConstPolygon3d & refer : refers) {
      bus_stop_area_polygon_ids_reg_elem.insert(refer.id());
    }
  }

  std::set<lanelet::Id> unreferenced_bus_stop_areas;
  std::set_difference(
    bus_stop_area_polygon_ids.begin(), bus_stop_area_polygon_ids.end(),
    bus_stop_area_polygon_ids_reg_elem.begin(), bus_stop_area_polygon_ids_reg_elem.end(),
    std::inserter(unreferenced_bus_stop_areas, unreferenced_bus_stop_areas.begin()));

  for (const auto & bus_stop_area_id : unreferenced_bus_stop_areas) {
    issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), bus_stop_area_id));
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
