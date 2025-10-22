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

#include "lanelet2_map_validator/validators/lane/walkway_intersection.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestWalkwayIntersectionValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::WalkwayIntersectionValidator::name());
};

TEST_F(TestWalkwayIntersectionValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::WalkwayIntersectionValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestWalkwayIntersectionValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::WalkwayIntersectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestWalkwayIntersectionValidator, WalkwayIntersection)  // NOLINT for gtest
{
  load_target_map("lane/walkway_intersection.osm");

  lanelet::autoware::validation::WalkwayIntersectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestWalkwayIntersectionValidator, WalkwayWithNoRoad)  // NOLINT for gtest
{
  load_target_map("lane/walkway_intersection_with_no_road.osm");

  lanelet::autoware::validation::WalkwayIntersectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 37);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestWalkwayIntersectionValidator, WalkwayWithShortExtension)  // NOLINT for gtest
{
  load_target_map("lane/walkway_intersection_with_short_walkway.osm");

  lanelet::autoware::validation::WalkwayIntersectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  std::map<std::string, std::string> reason_map;
  reason_map["road_lanelet_id"] = "49";
  reason_map["extension"] = "0.270767";
  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 2), 37, reason_map);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}
