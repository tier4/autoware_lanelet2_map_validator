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

#include "lanelet2_map_validator/validators/intersection/roundabout_configuration.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestRoundaboutConfigurationValidator : public MapValidationTester
{
protected:
  std::string test_target_ = "mapping.intersection.roundabout_configuration";
};

TEST_F(TestRoundaboutConfigurationValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RoundaboutConfigurationValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestRoundaboutConfigurationValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::RoundaboutConfigurationValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRoundaboutConfigurationValidator, ValidRoundabout)  // NOLINT for gtest
{
  load_target_map("intersection/roundabout.osm");

  lanelet::autoware::validation::RoundaboutConfigurationValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestRoundaboutConfigurationValidator, InvalidLaneletInPath)  // NOLINT for gtest
{
  load_target_map("intersection/roundabout_with_invalid_lanelet.osm");

  lanelet::autoware::validation::RoundaboutConfigurationValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 7);

  const lanelet::Id expected_reg_elem_id = 74253;

  lanelet::validation::Issues expected_issues;

  std::map<std::string, std::string> issue_001_1;
  issue_001_1["entry_lanelet_id"] = "18508";
  issue_001_1["exit_lanelet_id"] = "18378";
  issue_001_1["invalid_lanelet_id"] = "74260";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 1), expected_reg_elem_id, issue_001_1));

  std::map<std::string, std::string> issue_001_2;
  issue_001_2["entry_lanelet_id"] = "18508";
  issue_001_2["exit_lanelet_id"] = "18391";
  issue_001_2["invalid_lanelet_id"] = "74260";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 1), expected_reg_elem_id, issue_001_2));

  std::map<std::string, std::string> issue_001_3;
  issue_001_3["entry_lanelet_id"] = "18494";
  issue_001_3["exit_lanelet_id"] = "18391";
  issue_001_3["invalid_lanelet_id"] = "74260";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 1), expected_reg_elem_id, issue_001_3));

  std::map<std::string, std::string> issue_001_4;
  issue_001_4["entry_lanelet_id"] = "18522";
  issue_001_4["exit_lanelet_id"] = "18415";
  issue_001_4["invalid_lanelet_id"] = "74260";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 1), expected_reg_elem_id, issue_001_4));

  std::map<std::string, std::string> issue_001_5;
  issue_001_5["entry_lanelet_id"] = "18522";
  issue_001_5["exit_lanelet_id"] = "18378";
  issue_001_5["invalid_lanelet_id"] = "74260";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 1), expected_reg_elem_id, issue_001_5));

  std::map<std::string, std::string> issue_001_6;
  issue_001_6["entry_lanelet_id"] = "18522";
  issue_001_6["exit_lanelet_id"] = "18391";
  issue_001_6["invalid_lanelet_id"] = "74260";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 1), expected_reg_elem_id, issue_001_6));

  std::map<std::string, std::string> issue_002_map;
  issue_002_map["internal_lanelet_id"] = "18232";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 2), expected_reg_elem_id, issue_002_map));

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRoundaboutConfigurationValidator, MissingEnableExitTurnSignalTag)  // NOLINT for gtest
{
  load_target_map("intersection/roundabout_with_missing_exit_signal_tag.osm");

  lanelet::autoware::validation::RoundaboutConfigurationValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_reg_elem_id = 74253;

  lanelet::validation::Issues expected_issues;

  std::map<std::string, std::string> issue_003_map;
  issue_003_map["internal_lanelet_id"] = "18129";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 3), expected_reg_elem_id, issue_003_map));

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestRoundaboutConfigurationValidator, EnableExitTurnSignalTagFalse)  // NOLINT for gtest
{
  load_target_map("intersection/roundabout_with_non_conflict_tag_false.osm");

  lanelet::autoware::validation::RoundaboutConfigurationValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_reg_elem_id = 74253;

  lanelet::validation::Issues expected_issues;

  std::map<std::string, std::string> issue_004_map;
  issue_004_map["internal_lanelet_id"] = "18129";
  issue_004_map["tag_value"] = "false";
  expected_issues.push_back(
    construct_issue_from_code(issue_code(test_target_, 4), expected_reg_elem_id, issue_004_map));

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}
