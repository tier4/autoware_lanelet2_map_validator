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

#include "lanelet2_map_validator/validators/intersection/lanelet_division.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestLaneletDivisionValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::LaneletDivisionValidator::name());
};

TEST_F(TestLaneletDivisionValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::LaneletDivisionValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestLaneletDivisionValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::LaneletDivisionValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestLaneletDivisionValidator, LaneletDivisionInIntersection)  // NOLINT for gtest
{
  load_target_map("intersection/intersection_area_with_lanelet_division.osm");

  lanelet::autoware::validation::LaneletDivisionValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> substitution_map1;
  substitution_map1["successor_id"] = "57";
  substitution_map1["intersection_area_id"] = "10803";
  const auto expected_issue1 =
    construct_issue_from_code(issue_code(test_target_, 1), 10984, substitution_map1);

  std::map<std::string, std::string> substitution_map2;
  substitution_map2["successor_id"] = "10879";
  substitution_map2["intersection_area_id"] = "10803";
  const auto expected_issue2 =
    construct_issue_from_code(issue_code(test_target_, 1), 54, substitution_map2);

  const auto expected_issues = {expected_issue1, expected_issue2};

  EXPECT_EQ(issues.size(), 2);

  const auto difference = compare_issues(expected_issues, issues);
  EXPECT_TRUE(difference.empty()) << difference;
}
