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

#include "lanelet2_map_validator/validators/lane/pedestrian_lane.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

  class TestPedestrianLaneValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::PedestrianLaneValidator::name());
};

TEST_F(TestPedestrianLaneValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::PedestrianLaneValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestPedestrianLaneValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::PedestrianLaneValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestPedestrianLaneValidator, NoAdjacentLanelets)  // NOLINT for gtest
{
  load_target_map("lane/pedestrian_lane_without_adjacent.osm");

  lanelet::autoware::validation::PedestrianLaneValidator checker;
  const auto & issues = checker(*map_);

  ASSERT_FALSE(issues.empty());
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 24);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestPedestrianLaneValidator, NonRoadAdjacentLanelet)  // NOLINT for gtest
{
  load_target_map("lane/pedestrian_lane_with_non_road_adjacent.osm");

  lanelet::autoware::validation::PedestrianLaneValidator checker;
  const auto & issues = checker(*map_);

  ASSERT_FALSE(issues.empty());
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 2), 23);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestPedestrianLaneValidator, NoRoadBorderBound)  // NOLINT for gtest
{
  load_target_map("lane/pedestrian_lane_empty_side_no_road_border.osm");

  lanelet::autoware::validation::PedestrianLaneValidator checker;
  const auto & issues = checker(*map_);

  ASSERT_FALSE(issues.empty());
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 3), 19);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}
