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

#include "lanelet2_map_validator/validators/lane/border_sharing.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestBorderSharingValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::BorderSharingValidator::name());

  void expect_issue_pair(
    const lanelet::validation::Issues & issues, const int issue_number, const lanelet::Id id1,
    const lanelet::Id id2)
  {
    std::map<std::string, std::string> id_map;
    id_map["lanelet_id"] = std::to_string(id2);
    const auto expected_issue1 =
      construct_issue_from_code(issue_code(test_target_, issue_number), id1, id_map);
    id_map["lanelet_id"] = std::to_string(id1);
    const auto expected_issue2 =
      construct_issue_from_code(issue_code(test_target_, issue_number), id2, id_map);

    const auto expected_issues = {expected_issue1, expected_issue2};
    const auto difference = compare_issues(expected_issues, issues);
    EXPECT_TRUE(difference.empty()) << difference;
  }
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

  // The borders are not at the same position, so the lanelets are not judged as adjacent
  EXPECT_EQ(issues.size(), 0);
}

TEST_F(
  TestBorderSharingValidator, StraightLanesCompletelyDifferentJaggyBorders)  // NOLINT for gtest
{
  load_target_map("lane/straight_lanes_completely_different_jaggy_borders.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  // The borders are not at the same position, so the lanelets are not judged as adjacent
  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, StraightLanesDifferentBorderID)  // NOLINT for gtest
{
  load_target_map("lane/straight_lanes_different_border_id.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["lanelet_id"] = "10157";
  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 1), 108, id_map);
  id_map["lanelet_id"] = "108";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 1), 10157, id_map);

  const auto expected_issues = {expected_issue1, expected_issue2};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestBorderSharingValidator, CurveLanesCompletelyDifferentBorders)  // NOLINT for gtest
{
  load_target_map("lane/curve_lanes_completely_different_borders.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  // The borders are not at the same position, so the lanelets are not judged as adjacent
  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, CurveLanesCompletelyDifferentJaggyBorders)  // NOLINT for gtest
{
  load_target_map("lane/curve_lanes_completely_different_jaggy_borders.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  // The borders are not at the same position, so the lanelets are not judged as adjacent
  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, CurveLanesDifferentBorderID)  // NOLINT for gtest
{
  load_target_map("lane/curve_lanes_different_border_id.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> id_map;
  id_map["lanelet_id"] = "96";
  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 1), 7974, id_map);
  id_map["lanelet_id"] = "7974";
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 1), 96, id_map);

  const auto expected_issues = {expected_issue1, expected_issue2};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
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

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  // The borders are at the same position only in a part of their length, so the lanelets are not
  // judged as adjacent
  EXPECT_EQ(issues.size(), 0);
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

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  // The borders are at the same position only in a part of their length, so the lanelets are not
  // judged as adjacent
  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, SamePointsDifferentLinestrings)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_same_points.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  expect_issue_pair(issues, 1, 17, 18);
}

TEST_F(TestBorderSharingValidator, CopiedBorder)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_copied_points.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  expect_issue_pair(issues, 1, 21, 22);
}

TEST_F(TestBorderSharingValidator, SlightlyDifferentPoints)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_slightly_different_points.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, DifferentHeight)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_different_height.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, PartiallySamePoints)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_partially_same_points.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, BordersOnSameSide)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_same_side.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, OppositeLanesShared)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_opposite_lanes_shared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, OppositeLanesUnshared)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_opposite_lanes_unshared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  expect_issue_pair(issues, 1, 17, 18);
}

TEST_F(TestBorderSharingValidator, NoRoadLanelet)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_no_road.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, SharedBorderWithFence)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_fence_shared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  expect_issue_pair(issues, 3, 16, 17);
}

TEST_F(TestBorderSharingValidator, SharedFenceBorder)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_fence_typed_shared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  expect_issue_pair(issues, 3, 16, 17);
}

TEST_F(TestBorderSharingValidator, SharedBorderWithPartialFence)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_partial_fence_shared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  expect_issue_pair(issues, 3, 16, 17);
}

TEST_F(TestBorderSharingValidator, SharedBorderWithFenceRoadShoulder)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_shoulder_road_fence_shared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  expect_issue_pair(issues, 3, 16, 17);
}

TEST_F(TestBorderSharingValidator, UnsharedBordersWithFence)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_fence_unshared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, UnsharedFenceBorders)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_fence_typed_unshared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, UnsharedBordersWithPartialFence)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_partial_fence_unshared.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestBorderSharingValidator, UnsharedBordersWithFenceInsideLanelet)  // NOLINT for gtest
{
  load_target_map("lane/border_sharing_fence_inside_lanelet.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  expect_issue_pair(issues, 1, 17, 18);
}

TEST_F(TestBorderSharingValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::BorderSharingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
