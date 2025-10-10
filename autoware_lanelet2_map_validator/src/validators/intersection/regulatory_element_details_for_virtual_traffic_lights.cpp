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

#include "lanelet2_map_validator/validators/intersection/regulatory_element_details_for_virtual_traffic_lights.hpp"

#include "lanelet2_map_validator/utils.hpp"

#include <autoware_lanelet2_extension/regulatory_elements/virtual_traffic_light.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/BoundingBox.h>

#include <cmath>
#include <map>
#include <string>

namespace lanelet::autoware::validation
{
namespace
{
lanelet::validation::RegisterMapValidator<RegulatoryElementDetailsForVirtualTrafficLightsValidator>
  reg;
}

lanelet::validation::Issues RegulatoryElementDetailsForVirtualTrafficLightsValidator::operator()(
  const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  lanelet::autoware::validation::appendIssues(
    issues, check_regulatory_element_details_for_virtual_traffic_lights(map));

  return issues;
}

lanelet::validation::Issues RegulatoryElementDetailsForVirtualTrafficLightsValidator::
  check_regulatory_element_details_for_virtual_traffic_lights(const lanelet::LaneletMap & map)
{
  lanelet::validation::Issues issues;

  for (const auto & reg_elem : map.regulatoryElementLayer) {
    if (
      reg_elem->attributeOr(lanelet::AttributeName::Subtype, "") !=
      std::string(VirtualTrafficLight::RuleName)) {
      continue;
    }

    const lanelet::ConstLineStrings3d start_lines =
      reg_elem->getParameters<lanelet::ConstLineString3d>("start_line");
    /* No validation for the number of start_lines since it's already validated in the
     * virtual_traffic_light constructor */
    for (const lanelet::ConstLineString3d & start_line : start_lines) {
      if (
        start_line.attributeOr(lanelet::AttributeName::Type, "") !=
        std::string(lanelet::AttributeValueString::Virtual)) {
        issues.emplace_back(
          construct_issue_from_code(issue_code(this->name(), 1), start_line.id()));
      }
    }

    const lanelet::ConstLineStrings3d stop_lines =
      reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine);
    if (stop_lines.size() != 1) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 2), reg_elem->id()));
    }
    for (const lanelet::ConstLineString3d & stop_line : stop_lines) {
      if (
        stop_line.attributeOr(lanelet::AttributeName::Type, "") !=
        std::string(lanelet::AttributeValueString::StopLine)) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 3), stop_line.id()));
      }
    }

    const lanelet::ConstLineStrings3d end_lines =
      reg_elem->getParameters<lanelet::ConstLineString3d>("end_line");
    /* No validation for end_lines missing since it's already validated in the virtual_traffic_light
     * constructor */
    for (const lanelet::ConstLineString3d & end_line : end_lines) {
      if (
        end_line.attributeOr(lanelet::AttributeName::Type, "") !=
        std::string(lanelet::AttributeValueString::Virtual)) {
        issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 4), end_line.id()));
      }
    }

    const lanelet::ConstLineStrings3d refers_linestrings =
      reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers);
    if (refers_linestrings.size() == 0) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 5), reg_elem->id()));
    }
    for (const lanelet::ConstLineString3d & refers_linestring : refers_linestrings) {
      if (
        std::find(
          supported_refers_type_.begin(), supported_refers_type_.end(),
          refers_linestring.attributeOr(lanelet::AttributeName::Type, "")) ==
        supported_refers_type_.end()) {
        std::string supported_type_str;
        for (const auto & str : supported_refers_type_) {
          supported_type_str += str;
          if (str != supported_refers_type_.back()) {
            supported_type_str += ", ";
          }
        }
        std::map<std::string, std::string> supported_refers_map;
        supported_refers_map["supported_refers"] = supported_type_str;
        issues.emplace_back(construct_issue_from_code(
          issue_code(this->name(), 6), refers_linestring.id(), supported_refers_map));
      }
    }

    lanelet::BoundingBox2d bbox2d;

    for (const auto & start_line : start_lines) {
      for (const auto & point : start_line) {
        bbox2d.extend(point.basicPoint2d());
      }
    }

    for (const auto & stop_line : stop_lines) {
      for (const auto & point : stop_line) {
        bbox2d.extend(point.basicPoint2d());
      }
    }

    for (const auto & end_line : end_lines) {
      for (const auto & point : end_line) {
        bbox2d.extend(point.basicPoint2d());
      }
    }

    for (const auto & refers_linestring : refers_linestrings) {
      for (const auto & point : refers_linestring) {
        bbox2d.extend(point.basicPoint2d());
      }
    }

    double dx = bbox2d.max().x() - bbox2d.min().x();
    double dy = bbox2d.max().y() - bbox2d.min().y();
    double bounding_box_size = std::sqrt(dx * dx + dy * dy);

    if (bounding_box_size > max_bounding_box_size_) {
      issues.emplace_back(construct_issue_from_code(issue_code(this->name(), 7), reg_elem->id()));
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
