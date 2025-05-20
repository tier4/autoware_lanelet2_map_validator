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

#include "lanelet2_map_validator/validators/intersection/single_virtual_traffic_light_reference.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestSingleVirtualTrafficLightReferenceValidator : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::SingleVirtualTrafficLightReferenceValidator::name());
};

TEST_F(TestSingleVirtualTrafficLightReferenceValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::SingleVirtualTrafficLightReferenceValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(
  TestSingleVirtualTrafficLightReferenceValidator, CorrectVirtualTrafficLight)  // NOLINT for gtest
{
  load_target_map("intersection/correct_virtual_traffic_light.osm");

  lanelet::autoware::validation::SingleVirtualTrafficLightReferenceValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(
  TestSingleVirtualTrafficLightReferenceValidator,
  ReferringTwoVirtualTrafficLight)  // NOLINT for gtest
{
  load_target_map("intersection/referring_two_virtual_traffic_lights.osm");

  lanelet::autoware::validation::SingleVirtualTrafficLightReferenceValidator checker;
  const auto & issues = checker(*map_);

  const auto expected_issue = construct_issue_from_code(issue_code(test_target_, 1), 52);

  EXPECT_EQ(issues.size(), 1);

  const auto difference = compare_an_issue(expected_issue, issues[0]);
  EXPECT_TRUE(difference.empty()) << difference;
}

TEST_F(TestSingleVirtualTrafficLightReferenceValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::SingleVirtualTrafficLightReferenceValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
