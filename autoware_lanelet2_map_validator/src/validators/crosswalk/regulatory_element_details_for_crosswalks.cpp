// Copyright 2024 Autoware Foundation
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

#include "lanelet2_map_validator/validators/crosswalk/regulatory_element_details_for_crosswalks.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/crosswalk.hpp>
#include <range/v3/view/filter.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/BoundingBox.h>
#include <lanelet2_validation/Validation.h>

#include <cmath>
#include <map>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RegulatoryElementsDetailsForCrosswalksValidator> reg;
}  // namespace

lanelet::validation::Issues RegulatoryElementsDetailsForCrosswalksValidator::operator()(
  const lanelet::LaneletMap & map)
{
  // All issues found by all validators
  lanelet::validation::Issues issues;

  // Append issues found by each validator
  lanelet::autoware::validation::appendIssues(issues, checkRegulatoryElementOfCrosswalks(map));
  return issues;
}

lanelet::validation::Issues
RegulatoryElementsDetailsForCrosswalksValidator::checkRegulatoryElementOfCrosswalks(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;
  // filter elem whose Subtype is crosswalk
  auto elems = map.regulatoryElementLayer | ranges::views::filter([](auto && elem) {
                 const auto & attrs = elem->attributes();
                 const auto & it = attrs.find(lanelet::AttributeName::Subtype);
                 return it != attrs.end() && it->second == lanelet::AttributeValueString::Crosswalk;
               });

  for (const auto & elem : elems) {
    // Get lanelet of crosswalk referred by regulatory element
    auto refers = elem->getParameters<lanelet::ConstLanelet>(lanelet::RoleName::Refers);
    // Get stop line referred by regulatory element
    auto ref_lines = elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine);

    // Report error if regulatory element does not have lanelet of crosswalk
    if (refers.empty()) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 1), elem->id()));
    } else if (refers.size() > 1) {  // Report error if regulatory element has two or more lanelet
                                     // of crosswalk
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), elem->id()));
    }

    // Report Info if regulatory element does not have stop line
    if (ref_lines.empty()) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 3), elem->id()));
    } else if (ref_lines.size() > 1) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 9), elem->id()));
    }

    // If this is a crosswalk type regulatory element, the "refers" has to be a "crosswalk" subtype
    // lanelet
    const auto & issue_cw =
      construct_issue_from_code(issue_code(this->name(), 6), lanelet::utils::getId());
    lanelet::autoware::validation::checkPrimitivesType(
      refers, lanelet::AttributeValueString::Lanelet, lanelet::AttributeValueString::Crosswalk,
      issue_cw, issues);

    // The refers must have an attribute participant:pedestrian and set to "yes" or "true"
    for (const lanelet::ConstLanelet & lane : refers) {
      if (!lane.hasAttribute(lanelet::AttributeName::ParticipantPedestrian)) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 10), lane.id()));
      } else if (!lane.attribute(lanelet::AttributeName::ParticipantPedestrian)
                    .asBool()
                    .value_or(false)) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 11), lane.id()));
      }
    }

    // If this is a crosswalk type regulatory element, the "ref_line" has to be a "stop_line" type
    // linestring
    const auto & issue_sl =
      construct_issue_from_code(issue_code(this->name(), 7), lanelet::utils::getId());
    lanelet::autoware::validation::checkPrimitivesType(
      ref_lines, lanelet::AttributeValueString::StopLine, issue_sl, issues);

    lanelet::BoundingBox2d bbox2d;

    for (const auto & refer : refers) {
      for (const auto & point : refer.leftBound()) {
        bbox2d.extend(point.basicPoint2d());
      }

      for (const auto & point : refer.rightBound()) {
        bbox2d.extend(point.basicPoint2d());
      }
    }

    for (const auto & ref_line : ref_lines) {
      for (const auto & point : ref_line) {
        bbox2d.extend(point.basicPoint2d());
      }
    }

    auto polygons = elem->getParameters<lanelet::ConstPolygon3d>("crosswalk_polygon");
    for (const auto & polygon : polygons) {
      for (const auto & point : polygon) {
        bbox2d.extend(point.basicPoint2d());
      }
    }

    double dx = bbox2d.max().x() - bbox2d.min().x();
    double dy = bbox2d.max().y() - bbox2d.min().y();
    double bounding_box_size = std::sqrt(dx * dx + dy * dy);

    if (bounding_box_size > max_bounding_box_size_) {
      std::map<std::string, std::string> threshold_map;
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 12), elem->id()));
    }
  }
  return issues;
}

}  // namespace lanelet::autoware::validation
