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
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
          start_line.id(),
          append_issue_code_prefix(
            this->name(), 1,
            "The start_line of a virtual_traffic_light regulatory element must be a \"virtual\" "
            "type."));
      }
    }

    const lanelet::ConstLineStrings3d stop_lines =
      reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::RefLine);
    if (stop_lines.size() != 1) {
      issues.emplace_back(
        lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement,
        reg_elem->id(),
        append_issue_code_prefix(
          this->name(), 2,
          "A virtual_traffic_light regulatory element must only have a single ref_line."));
    }
    for (const lanelet::ConstLineString3d & stop_line : stop_lines) {
      if (
        stop_line.attributeOr(lanelet::AttributeName::Type, "") !=
        std::string(lanelet::AttributeValueString::StopLine)) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
          stop_line.id(),
          append_issue_code_prefix(
            this->name(), 3,
            "The ref_line of a virtual_traffic_light regulatory element must be a \"stop_line\" "
            "type."));
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
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
          end_line.id(),
          append_issue_code_prefix(
            this->name(), 4,
            "The end_line of a virtual_traffic_light regulatory element must be a \"virtual\" "
            "type."));
      }
    }

    const lanelet::ConstLineStrings3d coordinations =
      reg_elem->getParameters<lanelet::ConstLineString3d>(lanelet::RoleName::Refers);
    if (coordinations.size() == 0) {
      issues.emplace_back(
        lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement,
        reg_elem->id(),
        append_issue_code_prefix(
          this->name(), 5, "A virtual_traffic_light regulatory element must have a refers."));
    }
    for (const lanelet::ConstLineString3d & coordination : coordinations) {
      if (
        coordination.attributeOr(lanelet::AttributeName::Type, "") !=
        std::string("intersection_coordination")) {
        issues.emplace_back(
          lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString,
          coordination.id(),
          append_issue_code_prefix(
            this->name(), 6,
            "The refers of a virtual_traffic_light regulatory element must be an "
            "\"intersection_coordination\" type."));
      }
    }
  }

  return issues;
}
}  // namespace lanelet::autoware::validation
