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

#include "lanelet2_map_validator/validators/intersection/intersection_area_tagging.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestIntersectionAreaTaggingValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::IntersectionAreaTaggingValidator::name());
};

TEST_F(TestIntersectionAreaTaggingValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::IntersectionAreaTaggingValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestIntersectionAreaTaggingValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::IntersectionAreaTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestIntersectionAreaTaggingValidator, MissingIntersectionAreaTag)  // NOLINT for gtest
{
  load_target_map("intersection/intersection_area_without_intersection_area_tag.osm");

  lanelet::autoware::validation::IntersectionAreaTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 2);

  const lanelet::Id expected_lanelet_id = 49;
  const lanelet::Id expected_area_id = 10803;

  std::map<std::string, std::string> area_id_map;
  area_id_map["area_id"] = std::to_string(expected_area_id);
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 1), expected_lanelet_id, area_id_map);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_lanelet_id);
}

TEST_F(TestIntersectionAreaTaggingValidator, IncorrectIntersectionAreaTag)  // NOLINT for gtest
{
  load_target_map("intersection/intersection_area_with_wrong_intersection_area_id.osm");

  lanelet::autoware::validation::IntersectionAreaTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_lanelet_id = 49;
  const lanelet::Id expected_area_id = 10803;
  const lanelet::Id wrong_area_id = 2195;

  std::map<std::string, std::string> tag_map;
  tag_map["expected_area_id"] = std::to_string(expected_area_id);
  tag_map["actual_area_id"] = std::to_string(wrong_area_id);
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 2), expected_lanelet_id, tag_map);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
  EXPECT_EQ(issues[0].id, expected_lanelet_id);
}

TEST_F(
  TestIntersectionAreaTaggingValidator, LaneletNotCoveredByIntersectionArea)  // NOLINT for gtest
{
  load_target_map(
    "intersection/intersection_area_with_lanelet_not_covered_by_intersection_area.osm");

  lanelet::autoware::validation::IntersectionAreaTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 3);

  const lanelet::Id expected_lanelet_ids[3] = {52, 53, 54};
  const lanelet::Id expected_area_id = 10803;

  for (int i = 0; i < 3; i++) {
    std::map<std::string, std::string> area_id_map;
    area_id_map["area_id"] = std::to_string(expected_area_id);
    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 3), expected_lanelet_ids[i], area_id_map);

    bool found = false;
    for (const auto & issue : issues) {
      if (issue.id == expected_lanelet_ids[i]) {
        const auto difference = compare_an_issue(expected_issue, issue);
        EXPECT_TRUE(difference.empty()) << difference;
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found) << "No issue found for lanelet ID " << expected_lanelet_ids[i];
  }
}
