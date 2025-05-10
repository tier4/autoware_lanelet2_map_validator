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
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <algorithm>
#include <string>

class TestRegulatoryElementDetailsForVirtualTrafficLightsValidator : public MapValidationTester
{
private:
};

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  CorrectVirtualTrafficLight)  // NOLINT for gtest
{
  load_target_map("intersection/correct_virtual_traffic_light.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  MissingStartLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_missing_start_line.osm");

  // virtual_traffic_light-type regulatory elements that has no start lines will not be loaded from
  // the start and should be mentioned in the loading_errors

  int target_primitive_id = 11074;
  const std::string target_message =
    "Error parsing primitive " + std::to_string(target_primitive_id) +
    ": Creating a regulatory element of type virtual_traffic_light failed: There must be exactly "
    "one start_line defined!";

  bool found_error_on_loading = std::any_of(
    loading_errors_.begin(), loading_errors_.end(),
    [&](const std::string & error) { return error.find(target_message) != std::string::npos; });

  EXPECT_TRUE(found_error_on_loading);
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  MultipleStartLines)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_multiple_start_lines.osm");

  // virtual_traffic_light-type regulatory elements that has multiple start lines will not be loaded
  // from the start and should be mentioned in the loading_errors

  int target_primitive_id = 11074;
  const std::string target_message =
    "Error parsing primitive " + std::to_string(target_primitive_id) +
    ": Creating a regulatory element of type virtual_traffic_light failed: There must be exactly "
    "one start_line defined!";

  bool found_error_on_loading = std::any_of(
    loading_errors_.begin(), loading_errors_.end(),
    [&](const std::string & error) { return error.find(target_message) != std::string::npos; });

  EXPECT_TRUE(found_error_on_loading);
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  WrongStartLineType)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_wrong_start_line_type.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11070,
    "[Intersection.RegulatoryElementDetailsForVirtualTrafficLights-001] The start_line of a "
    "virtual_traffic_light regulatory element must be a \"virtual\" type.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator, MissingRefLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_missing_ref_line.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.RegulatoryElementDetailsForVirtualTrafficLights-002] A virtual_traffic_light "
    "regulatory element must only have a single ref_line.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  MultipleRefLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_multiple_ref_lines.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.RegulatoryElementDetailsForVirtualTrafficLights-002] A virtual_traffic_light "
    "regulatory element must only have a single ref_line.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  WrongRefLineType)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_wrong_ref_line_type.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 378,
    "[Intersection.RegulatoryElementDetailsForVirtualTrafficLights-003] The ref_line of a "
    "virtual_traffic_light regulatory element must be a \"stop_line\" type.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator, MissingEndLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_missing_end_line.osm");

  // virtual_traffic_light-type regulatory elements that has no end lines will not be loaded from
  // the start and should be mentioned in the loading_errors

  int target_primitive_id = 11074;
  const std::string target_message =
    "Error parsing primitive " + std::to_string(target_primitive_id) +
    ": Creating a regulatory element of type virtual_traffic_light failed: No end_line defined!";

  bool found_error_on_loading = std::any_of(
    loading_errors_.begin(), loading_errors_.end(),
    [&](const std::string & error) { return error.find(target_message) != std::string::npos; });

  EXPECT_TRUE(found_error_on_loading);
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  WrongEndLineType)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_wrong_end_line_type.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11086,
    "[Intersection.RegulatoryElementDetailsForVirtualTrafficLights-004] The end_line of a "
    "virtual_traffic_light regulatory element must be a \"virtual\" type.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator, MissingRefers)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_missing_refers.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.RegulatoryElementDetailsForVirtualTrafficLights-005] A virtual_traffic_light "
    "regulatory element must have a refers.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(
  TestRegulatoryElementDetailsForVirtualTrafficLightsValidator,
  WrongRefersType)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_wrong_refers_type.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11073,
    "[Intersection.RegulatoryElementDetailsForVirtualTrafficLights-006] The refers of a "
    "virtual_traffic_light regulatory element must be an \"intersection_coordination\" type.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(TestRegulatoryElementDetailsForVirtualTrafficLightsValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RegulatoryElementDetailsForVirtualTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
