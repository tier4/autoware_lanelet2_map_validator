// Copyright 2024-2025 TIER IV, Inc.
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

#include "lanelet2_map_validator/validators/intersection/intersection_area_dangling_reference.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestIntersectionAreaDanglingReference : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::IntersectionAreaDanglingReferenceValidator::name());
};

TEST_F(TestIntersectionAreaDanglingReference, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::IntersectionAreaDanglingReferenceValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestIntersectionAreaDanglingReference, ValidateDanglingReference)  // NOLINT for gtest
{
  load_target_map("intersection/intersection_area_with_dangling_reference.osm");
  lanelet::autoware::validation::IntersectionAreaDanglingReferenceValidator checker;
  const auto & issues = checker(*map_);

  std::map<std::string, std::string> area_id_map;
  area_id_map["area_id"] = "777";
  const auto expected_issue =
    construct_issue_from_code(issue_code(test_target_, 1), 53, area_id_map);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestIntersectionAreaDanglingReference, ValidIntersectionArea)  // NOLINT for gtest
{
  load_target_map("intersection/basic_intersection_area.osm");
  lanelet::autoware::validation::IntersectionAreaDanglingReferenceValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
