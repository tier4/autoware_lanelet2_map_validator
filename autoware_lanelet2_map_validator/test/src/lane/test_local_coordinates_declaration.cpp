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

#include "lanelet2_map_validator/validators/lane/local_coordinates_declaration.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestLocalCoordinatesDeclarationValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::LocalCoordinatesDeclarationValidator::name());
};

TEST_F(TestLocalCoordinatesDeclarationValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::LocalCoordinatesDeclarationValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestLocalCoordinatesDeclarationValidator, NotLocalFromStart)  // NOLINT for gtest
{
  load_target_map("lane/single_lanelet_first_three_points_missing_local_coordinates.osm");

  lanelet::autoware::validation::LocalCoordinatesDeclarationValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 1), 8);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 1), 9);
  const auto expected_issue3 = construct_issue_from_code(issue_code(test_target_, 1), 11);
  const auto expected_issues = {expected_issue1, expected_issue2, expected_issue3};

  const auto difference = compare_issues(expected_issues, issues);

  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestLocalCoordinatesDeclarationValidator, PartiallyDefected)  // NOLINT for gtest
{
  load_target_map("lane/single_lanelet_partially_defected_local_coordinates.osm");

  lanelet::autoware::validation::LocalCoordinatesDeclarationValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue1 = construct_issue_from_code(issue_code(test_target_, 2), 8);
  const auto expected_issue2 = construct_issue_from_code(issue_code(test_target_, 3), 17);
  const auto expected_issues = {expected_issue1, expected_issue2};

  const auto difference = compare_issues(expected_issues, issues);

  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(
  TestLocalCoordinatesDeclarationValidator, NormalLaneletWithLocalCoordinates)  // NOLINT for gtest
{
  load_target_map("lane/single_lanelet.osm");

  lanelet::autoware::validation::LocalCoordinatesDeclarationValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(
  TestLocalCoordinatesDeclarationValidator,
  NormalLaneletWithoutLocalCoordinates)  // NOLINT for gtest
{
  load_target_map("lane/single_lanelet_without_local_coordinates.osm");

  lanelet::autoware::validation::LocalCoordinatesDeclarationValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestLocalCoordinatesDeclarationValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::LocalCoordinatesDeclarationValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
