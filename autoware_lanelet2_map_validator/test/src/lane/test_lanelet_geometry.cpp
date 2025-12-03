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

#include "lanelet2_map_validator/validators/lane/lanelet_geometry.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestLaneletGeometryValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::LaneletGeometryValidator::name());
};

TEST_F(TestLaneletGeometryValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::LaneletGeometryValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestLaneletGeometryValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::LaneletGeometryValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestLaneletGeometryValidator, SharedPointsBetweenBounds)  // NOLINT for gtest
{
  load_target_map("lane/lanelet_bound_with_sharing_point.osm");

  lanelet::autoware::validation::LaneletGeometryValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 152);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestLaneletGeometryValidator, IsolationForestAnomalyDetection)  // NOLINT for gtest
{
  load_target_map("lane/lanelet_geometry_check_1.osm");

  lanelet::autoware::validation::LaneletGeometryValidator checker;
  const auto & issues = checker(*map_);

  // Expected lanelet IDs and their approximate scores based on the output
  std::map<int, double> expected_anomaly_lanelets = {
    {125, 0.633220}, {124, 0.625706}, {50, 0.694877}, {53, 0.679235},
    {57, 0.710649},  {60, 0.675717},  {54, 0.733188}};

  // Create expected issues with substitution maps
  lanelet::validation::Issues expected_issues;
  for (const auto & [lanelet_id, score] : expected_anomaly_lanelets) {
    std::map<std::string, std::string> substitution_map;
    substitution_map["anomaly_score"] = std::to_string(score);
    substitution_map["threshold"] = "0.620000";

    expected_issues.emplace_back(
      construct_issue_from_code(issue_code(test_target_, 2), lanelet_id, substitution_map));
  }

  // Compare issues
  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestLaneletGeometryValidator, IsolationForestAnomalyDetectionCheck2)  // NOLINT for gtest
{
  load_target_map("lane/lanelet_geometry_check_2.osm");

  lanelet::autoware::validation::LaneletGeometryValidator checker;
  const auto & issues = checker(*map_);

  // Expected lanelet IDs and their approximate scores based on the output
  std::map<int, double> expected_anomaly_lanelets = {{49, 0.688922}, {50, 0.664310}, {52, 0.685694},
                                                     {53, 0.658740}, {54, 0.699378}, {57, 0.634708},
                                                     {59, 0.668245}, {60, 0.655135}};

  // Create expected issues with substitution maps
  lanelet::validation::Issues expected_issues;
  for (const auto & [lanelet_id, score] : expected_anomaly_lanelets) {
    std::map<std::string, std::string> substitution_map;
    substitution_map["anomaly_score"] = std::to_string(score);
    substitution_map["threshold"] = "0.620000";

    expected_issues.emplace_back(
      construct_issue_from_code(issue_code(test_target_, 2), lanelet_id, substitution_map));
  }

  // Compare issues
  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestLaneletGeometryValidator, IsolationForestAnomalyDetectionCheck3)  // NOLINT for gtest
{
  load_target_map("lane/lanelet_geometry_check_3.osm");

  lanelet::autoware::validation::LaneletGeometryValidator checker;
  const auto & issues = checker(*map_);

  // Expected lanelet IDs and their approximate scores based on the output
  std::map<int, double> expected_anomaly_lanelets = {
    {15, 0.656861}, {18, 0.669237}, {9463, 0.707636}, {9102, 0.693157}, {10291, 0.719456}};

  // Create expected issues with substitution maps
  lanelet::validation::Issues expected_issues;
  for (const auto & [lanelet_id, score] : expected_anomaly_lanelets) {
    std::map<std::string, std::string> substitution_map;
    substitution_map["anomaly_score"] = std::to_string(score);
    substitution_map["threshold"] = "0.620000";

    expected_issues.emplace_back(
      construct_issue_from_code(issue_code(test_target_, 2), lanelet_id, substitution_map));
  }

  // Compare issues
  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}
