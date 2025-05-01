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

#include "lanelet2_map_validator/validators/intersection/virtual_traffic_light_line_order.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestVirtualTrafficLightLineOrderValidator : public MapValidationTester
{
private:
};

TEST_F(TestVirtualTrafficLightLineOrderValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, DerailedStartLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_derailed_start_line.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11101,
    "[Intersection.VirtualTrafficLightLineOrder-001] The start_line isn't placed on a lanelet.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, EndLineCoveringWrongLanelet)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_end_line_covering_wrong_lanelet.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-002] Cannot find the end_line for referrer lanelet "
    "52. The end_line should be on the referrer lanelet.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, DerailedEndLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_derailed_end_line.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-002] Cannot find the end_line for referrer lanelet "
    "52. The end_line should be on the referrer lanelet.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, StartLineCoveringReferrer)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_start_line_covering_referrer.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11101,
    "[Intersection.VirtualTrafficLightLineOrder-003] The start_line must not intersect with the "
    "referrer lanelet of the virtual traffic light.");

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, InvalidPath)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_invalid_path.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-004] Cannot find a lanelet path from start_line to "
    "end_line.");

  // There will be three same issues since this regulatory element is referred three times.
  const lanelet::validation::Issues expected_issues = {
    expected_issue, expected_issue, expected_issue};

  EXPECT_EQ(issues.size(), 3);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, DerailedStopLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_derailed_stop_line.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11104,
    "[Intersection.VirtualTrafficLightLineOrder-005] The stop_line seems not to be placed on the "
    "path from start_line to end_line.");

  // There will be three same issues since this regulatory element is referred three times.
  const lanelet::validation::Issues expected_issues = {
    expected_issue, expected_issue, expected_issue};

  EXPECT_EQ(issues.size(), 3);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, StopLineNotOnPath)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_stop_line_out_of_path.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11107,
    "[Intersection.VirtualTrafficLightLineOrder-005] The stop_line seems not to be placed on the "
    "path from start_line to end_line.");

  // There will be three same issues since this regulatory element is referred three times.
  const lanelet::validation::Issues expected_issues = {
    expected_issue, expected_issue, expected_issue};

  EXPECT_EQ(issues.size(), 3);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, MessyStopLine)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_with_messy_stop_line.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-006] The order of start line, stop line, and end "
    "line is wrong (end_line ID: 11086).");
  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-006] The order of start line, stop line, and end "
    "line is wrong (end_line ID: 11089).");
  const lanelet::validation::Issue expected_issue3(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-006] The order of start line, stop line, and end "
    "line is wrong (end_line ID: 11091).");

  // There will be three same issues since this regulatory element is referred three times.
  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  EXPECT_EQ(issues.size(), 3);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, OrderedStopStartEnd)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_ordered_stop_start_end.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-006] The order of start line, stop line, and end "
    "line is wrong (end_line ID: 11086).");
  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-006] The order of start line, stop line, and end "
    "line is wrong (end_line ID: 11089).");
  const lanelet::validation::Issue expected_issue3(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-006] The order of start line, stop line, and end "
    "line is wrong (end_line ID: 11091).");

  // There will be three same issues since this regulatory element is referred three times.
  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  EXPECT_EQ(issues.size(), 3);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, OrderedStartEndStop)  // NOLINT for gtest
{
  load_target_map("intersection/virtual_traffic_light_ordered_start_end_stop.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::RegulatoryElement, 11074,
    "[Intersection.VirtualTrafficLightLineOrder-006] The order of start line, stop line, and end "
    "line is wrong (end_line ID: 11089).");
  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11099,
    "[Intersection.VirtualTrafficLightLineOrder-005] The stop_line seems not to be placed on the "
    "path from start_line to end_line.");
  const lanelet::validation::Issue expected_issue3(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::LineString, 11099,
    "[Intersection.VirtualTrafficLightLineOrder-005] The stop_line seems not to be placed on the "
    "path from start_line to end_line.");

  // There will be three same issues since this regulatory element is referred three times.
  const lanelet::validation::Issues expected_issues = {
    expected_issue1, expected_issue2, expected_issue3};

  EXPECT_EQ(issues.size(), 3);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, CorrectVirtualTrafficLight)  // NOLINT for gtest
{
  load_target_map("intersection/correct_virtual_traffic_light.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestVirtualTrafficLightLineOrderValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::VirtualTrafficLightLineOrderValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
