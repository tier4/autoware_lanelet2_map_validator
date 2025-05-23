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

#include "lanelet2_map_validator/validators/lane/longitudinal_subtype_connection.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestLongitudinalSubtypeConnectionValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::LongitudinalSubtypeConnectionValidator::name());
};

TEST_F(TestLongitudinalSubtypeConnectionValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::LongitudinalSubtypeConnectionValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(
  TestLongitudinalSubtypeConnectionValidator, CorrectLongitudinalConnection)  // NOLINT for gtest
{
  load_target_map("lane/correct_longitudinal_lanelet_subtype_connection.osm");

  lanelet::autoware::validation::LongitudinalSubtypeConnectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestLongitudinalSubtypeConnectionValidator, SplitCrosswalkAndWalkway)  // NOLINT for gtest
{
  load_target_map("lane/split_crosswalk_and_walkway.osm");

  lanelet::autoware::validation::LongitudinalSubtypeConnectionValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 1), 11028);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 1), 11057);
  const auto expected_issues = {expected_issue1, expected_issue2};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestLongitudinalSubtypeConnectionValidator, WrongLongitudinalConnection)  // NOLINT for gtest
{
  load_target_map("lane/wrong_longitudinal_lanelet_subtype_connection.osm");

  lanelet::autoware::validation::LongitudinalSubtypeConnectionValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 2), 10303);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 2), 11050);
  const auto expected_issue3 = construct_issue_from_code(issue_code(test_target_, 2), 9178);
  const auto expected_issues = {expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestLongitudinalSubtypeConnectionValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::LongitudinalSubtypeConnectionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
