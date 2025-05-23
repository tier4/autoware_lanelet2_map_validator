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

#include "lanelet2_map_validator/validators/traffic_light/traffic_light_facing.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestTrafficLightFacing : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::TrafficLightFacingValidator::name());
};

TEST_F(TestTrafficLightFacing, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::TrafficLightFacingValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestTrafficLightFacing, WrongReferrerLanelet)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_wrong_referrer_lanelet.osm");

  lanelet::autoware::validation::TrafficLightFacingValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 416);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestTrafficLightFacing, WrongTrafficLightFacing)  // NOLINT for gtest
{
  load_target_map("traffic_light/crosswalk_with_wrong_traffic_light_facing.osm");

  lanelet::autoware::validation::TrafficLightFacingValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 2), 48);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestTrafficLightFacing, UncertainTrafficLightFacing)  // NOLINT for gtest
{
  load_target_map("traffic_light/crosswalk_with_entirely_synched_traffic_lights.osm");

  lanelet::autoware::validation::TrafficLightFacingValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 3), 48);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 3), 62);
  const auto expected_issue3 = construct_issue_from_code(issue_code(test_target_, 3), 74);
  const auto expected_issue4 = construct_issue_from_code(issue_code(test_target_, 3), 76);
  const auto expected_issues = {expected_issue1, expected_issue2, expected_issue3, expected_issue4};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestTrafficLightFacing, CorrectFacing)  // NOLINT for gtest
{
  load_target_map("traffic_light/crosswalk_with_traffic_lights.osm");

  lanelet::autoware::validation::TrafficLightFacingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestTrafficLightFacing, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::TrafficLightFacingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);  // Four INFOs should appear
}
