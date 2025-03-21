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

#include "lanelet2_map_validator/validators/lane/border_sharing.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestBorderSharingValidator : public MapValidationTester
{
private:
};

TEST_F(TestBorderSharingValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::BorderSharingValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestBorderSharingValidator, CorrectBorderSharing)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_base.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, StraightLanesCompletelyDifferentBorders)  // NOLINT for gtest
{
  load_target_map("lane/straight_lanes_completely_different_borders.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 108,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 10157 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 10157,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 108 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issues expected_issues = {expected_issue1, expected_issue2};

  EXPECT_EQ(issues.size(), 2);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(
  TestBorderSharingValidator, StraightLanesCompletelyDifferentJaggyBorders)  // NOLINT for gtest
{
  load_target_map("lane/straight_lanes_completely_different_jaggy_borders.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 108,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 10157 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 10157,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 108 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issues expected_issues = {expected_issue1, expected_issue2};

  EXPECT_EQ(issues.size(), 2);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestBorderSharingValidator, StraightLanesDifferentBorderID)  // NOLINT for gtest
{
  load_target_map("lane/straight_lanes_different_border_id.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 108,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 10157 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 10157,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 108 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issues expected_issues = {expected_issue1, expected_issue2};

  EXPECT_EQ(issues.size(), 2);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestBorderSharingValidator, CurveLanesCompletelyDifferentBorders)  // NOLINT for gtest
{
  load_target_map("lane/curve_lanes_completely_different_borders.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 96,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 7974 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 7974,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 96 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issues expected_issues = {expected_issue1, expected_issue2};

  EXPECT_EQ(issues.size(), 2);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestBorderSharingValidator, CurveLanesCompletelyDifferentJaggyBorders)  // NOLINT for gtest
{
  load_target_map("lane/curve_lanes_completely_different_jaggy_borders.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 96,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 7974 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 7974,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 96 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issues expected_issues = {expected_issue1, expected_issue2};

  EXPECT_EQ(issues.size(), 2);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestBorderSharingValidator, CurveLanesDifferentBorderID)  // NOLINT for gtest
{
  load_target_map("lane/curve_lanes_different_border_id.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  const lanelet::validation::Issue expected_issue1(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 96,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 7974 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issue expected_issue2(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 7974,
    "[Lane.BorderSharing-001] Seems to be adjacent with Lanelet 96 but doesn't share a border "
    "linestring.");

  const lanelet::validation::Issues expected_issues = {expected_issue1, expected_issue2};

  EXPECT_EQ(issues.size(), 2);
  EXPECT_TRUE(are_same_issues(issues, expected_issues));
}

TEST_F(TestBorderSharingValidator, PseudoBidirectionalLanes)  // NOLINT for gtest
{
  load_target_map("lane/pseudo_bidirectional_lanes.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, WrongLaneletDivision)  // NOLINT for gtest
{
  load_target_map("lane/wrong_lanelet_division.osm");

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 108,
    "[Lane.BorderSharing-002] Seems to be adjacent with Lanelets 107 7944 but doesn't share a "
    "border "
    "linestring.");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(TestBorderSharingValidator, CorrectBorderSharingLaneSplit)  // NOLINT for gtest
{
  load_target_map("lane/correct_border_sharing_lane_split.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, WrongBorderSharingLaneSplit)  // NOLINT for gtest
{
  load_target_map("lane/wrong_border_sharing_lane_split.osm");

  const lanelet::validation::Issue expected_issue(
    lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, 108,
    "[Lane.BorderSharing-002] Seems to be adjacent with Lanelets 107 but doesn't share a border "
    "linestring.");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_TRUE(is_same_issue(issues[0], expected_issue));
}

TEST_F(TestBorderSharingValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
