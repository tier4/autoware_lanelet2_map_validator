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

#include "lanelet2_map_validator/validators/area/no_parking_area.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>

#include <algorithm>
#include <set>
#include <unordered_set>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<NoParkingAreaValidator> reg;
}

lanelet::validation::Issues NoParkingAreaValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_no_parking_area(map));

  return issues;
}

lanelet::validation::Issues NoParkingAreaValidator::check_no_parking_area(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  std::set<lanelet::Id> no_parking_area_polygon_ids;
  for (const auto & polygon : map.polygonLayer) {
    if (
      polygon.hasAttribute(lanelet::AttributeName::Type) &&
      polygon.attribute(lanelet::AttributeName::Type) == "no_parking_area") {
      no_parking_area_polygon_ids.insert(polygon.id());
    }
  }

  std::set<lanelet::Id> referenced_no_parking_area_polygon_ids;

  for (const auto & reg_elem : map.regulatoryElementLayer) {
    if (
      reg_elem->hasAttribute(lanelet::AttributeName::Subtype) &&
      reg_elem->attribute(lanelet::AttributeName::Subtype) == "no_parking_area") {
      // Issue-002: Regulatory element should refer to exactly one polygon
      const auto & refers =
        reg_elem->getParameters<lanelet::ConstPolygon3d>(lanelet::RoleName::Refers);
      if (refers.size() != 1) {
        for (const auto & poly : refers) {
          if (
            poly.hasAttribute(lanelet::AttributeName::Type) &&
            poly.attribute(lanelet::AttributeName::Type) == "no_parking_area") {
            referenced_no_parking_area_polygon_ids.insert(poly.id());
          }
        }
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), reg_elem->id()));
        continue;
      }

      // Issue-003: The referred polygon should be a no_parking_area type
      const auto & polygon = refers[0];
      if (
        !polygon.hasAttribute(lanelet::AttributeName::Type) ||
        polygon.attribute(lanelet::AttributeName::Type) != "no_parking_area") {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 3), reg_elem->id()));
        continue;
      }

      referenced_no_parking_area_polygon_ids.insert(polygon.id());

      // Issue-004: Regulatory element should be referred by at least one lanelet
      const auto referrers = map.laneletLayer.findUsages(reg_elem);
      if (referrers.empty()) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 4), reg_elem->id()));
      }

      // Issue-005: Check if there are non-road referrers (should only be referred by road lanelets)
      if (
        !referrers.empty() &&
        std::any_of(referrers.begin(), referrers.end(), [](lanelet::ConstLanelet lane) {
          return !lane.hasAttribute(lanelet::AttributeName::Subtype) ||
                 lane.attribute(lanelet::AttributeName::Subtype) !=
                   lanelet::AttributeValueString::Road;
        })) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 5), reg_elem->id()));
      }
    }
  }

  // Issue-001: Find no_parking_area polygons not referred by any regulatory element
  std::set<lanelet::Id> unreferenced_no_parking_areas;
  std::set_difference(
    no_parking_area_polygon_ids.begin(), no_parking_area_polygon_ids.end(),
    referenced_no_parking_area_polygon_ids.begin(), referenced_no_parking_area_polygon_ids.end(),
    std::inserter(unreferenced_no_parking_areas, unreferenced_no_parking_areas.begin()));

  for (const auto & polygon_id : unreferenced_no_parking_areas) {
    issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), polygon_id));
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
