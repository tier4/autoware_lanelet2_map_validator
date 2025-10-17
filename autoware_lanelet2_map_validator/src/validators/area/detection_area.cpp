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

#include "lanelet2_map_validator/validators/area/detection_area.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <lanelet2_core/LaneletMap.h>

#include <algorithm>
#include <set>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<DetectionAreaValidator> reg;
}

lanelet::validation::Issues DetectionAreaValidator::operator()(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(issues, check_detection_area(map));

  return issues;
}

lanelet::validation::Issues DetectionAreaValidator::check_detection_area(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  std::set<lanelet::Id> detection_area_polygon_ids;
  for (const auto & polygon : map.polygonLayer) {
    if (
      polygon.hasAttribute(lanelet::AttributeName::Type) &&
      polygon.attribute(lanelet::AttributeName::Type) == "detection_area") {
      detection_area_polygon_ids.insert(polygon.id());
    }
  }

  std::set<lanelet::Id> referenced_detection_area_polygon_ids;

  for (const auto & reg_elem : map.regulatoryElementLayer) {
    if (
      reg_elem->hasAttribute(lanelet::AttributeName::Subtype) &&
      reg_elem->attribute(lanelet::AttributeName::Subtype) == "detection_area") {
      // Issue-002: Regulatory element should refer to exactly one detection_area polygon
      const auto & refers =
        reg_elem->getParameters<lanelet::ConstPolygon3d>(lanelet::RoleName::Refers);
      if (refers.size() != 1) {
        for (const auto & poly : refers) {
          if (
            poly.hasAttribute(lanelet::AttributeName::Type) &&
            poly.attribute(lanelet::AttributeName::Type) == "detection_area") {
            referenced_detection_area_polygon_ids.insert(poly.id());
          }
        }
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), reg_elem->id()));
        continue;
      }

      // Issue-003: The referred polygon should be a detection_area type
      const auto & polygon = refers[0];
      if (
        !polygon.hasAttribute(lanelet::AttributeName::Type) ||
        polygon.attribute(lanelet::AttributeName::Type) != "detection_area") {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 3), reg_elem->id()));
        continue;
      }

      referenced_detection_area_polygon_ids.insert(polygon.id());

      // Issue-004: Regulatory element should refer to exactly one stop_line type ref_line
      const auto & ref_lines =
        reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine);
      if (
        ref_lines.size() != 1 || !ref_lines[0].hasAttribute(lanelet::AttributeName::Type) ||
        ref_lines[0].attribute(lanelet::AttributeName::Type) !=
          lanelet::AttributeValueString::StopLine) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 4), reg_elem->id()));
        continue;
      }

      // Issue-005: Regulatory element should be referred by at least one lanelet
      const auto referrers = map.laneletLayer.findUsages(reg_elem);
      if (referrers.empty()) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 5), reg_elem->id()));
      }

      // Issue-006: Check if there are non-road referrers (should only be referred by road lanelets)
      if (
        !referrers.empty() &&
        std::any_of(referrers.begin(), referrers.end(), [](lanelet::ConstLanelet lane) {
          return !lane.hasAttribute(lanelet::AttributeName::Subtype) ||
                 lane.attribute(lanelet::AttributeName::Subtype) !=
                   lanelet::AttributeValueString::Road;
        })) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 6), reg_elem->id()));
      }
    }
  }

  // Issue-001: Find detection_area polygons not referred by any regulatory element
  std::set<lanelet::Id> unreferenced_detection_areas;
  std::set_difference(
    detection_area_polygon_ids.begin(), detection_area_polygon_ids.end(),
    referenced_detection_area_polygon_ids.begin(), referenced_detection_area_polygon_ids.end(),
    std::inserter(unreferenced_detection_areas, unreferenced_detection_areas.begin()));

  for (const auto & polygon_id : unreferenced_detection_areas) {
    issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), polygon_id));
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
