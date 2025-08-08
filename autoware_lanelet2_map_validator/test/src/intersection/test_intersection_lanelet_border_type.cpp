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

#include "lanelet2_map_validator/utils.hpp"
#include "lanelet2_map_validator/validators/intersection/intersection_lanelet_border_type.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestIntersectionLaneletBorderTypeValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::IntersectionLaneletBorderTypeValidator::name());
};

TEST_F(TestIntersectionLaneletBorderTypeValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::IntersectionLaneletBorderTypeValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestIntersectionLaneletBorderTypeValidator, InvalidBorderType)  // NOLINT for gtest
{
  load_target_map("intersection/intersection_lanelet_invalid_border_type.osm");

  lanelet::autoware::validation::IntersectionLaneletBorderTypeValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);

  const lanelet::Id expected_lanelet_id_1 = 13;
  const auto expected_issue_1 =
    construct_issue_from_code(issue_code(test_target_, 1), expected_lanelet_id_1);

  bool found_lanelet_13 = false;
  for (const auto & issue : issues) {
    if (issue.id == expected_lanelet_id_1) {
      found_lanelet_13 = true;
      const auto difference = compare_an_issue(expected_issue_1, issue);
      EXPECT_TRUE(difference.empty()) << difference;
      EXPECT_EQ(issue.id, expected_lanelet_id_1) << "Issue should be for lanelet ID 13";
      break;
    }
  }
  EXPECT_TRUE(found_lanelet_13) << "Should find issue for lanelet 13";
}

TEST_F(TestIntersectionLaneletBorderTypeValidator, ValidConfiguration)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::IntersectionLaneletBorderTypeValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0)
    << "sample_map.osm should pass intersection lanelet border type validation";
}
