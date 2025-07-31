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

#include "lanelet2_map_validator/utils.hpp"
#include "lanelet2_map_validator/validators/intersection/right_of_way_with_traffic_lights.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <map>
#include <string>

class TestRightOfWayWithTrafficLights : public MapValidationTester
{
protected:
  const std::string test_target_ =
    std::string(lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator::name());
};

TEST_F(TestRightOfWayWithTrafficLights, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestRightOfWayWithTrafficLights, MissingRightOfWayReference)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_missing_reference.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);
  EXPECT_GT(issues.size(), 0) << "Should find issues for missing right_of_way references";
  for (const auto & issue : issues) {
    std::map<std::string, std::string> reason_map;
    auto lanelet = map_->laneletLayer.get(issue.id);
    reason_map["turn_direction"] = lanelet.attribute("turn_direction").value();

    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 1), issue.id, reason_map);
    const auto difference = compare_an_issue(expected_issue, issue);
    EXPECT_TRUE(difference.empty()) << difference;
  }
}

TEST_F(TestRightOfWayWithTrafficLights, WrongRightOfWay)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way_with_wrong_role_lanelet_reference.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_GT(issues.size(), 0) << "Should find issues for wrong right_of_way assignments";
  for (const auto & issue : issues) {
    std::map<std::string, std::string> reason_map;
    auto lanelet = map_->laneletLayer.get(issue.id);
    reason_map["turn_direction"] = lanelet.attribute("turn_direction").value();

    for (const auto & reg_elem : lanelet.regulatoryElements()) {
      if (
        reg_elem->hasAttribute("subtype") &&
        reg_elem->attribute("subtype").value() == "right_of_way") {
        reason_map["right_of_way_id"] = std::to_string(reg_elem->id());
        break;
      }
    }

    const auto expected_issue =
      construct_issue_from_code(issue_code(test_target_, 2), issue.id, reason_map);
    const auto difference = compare_an_issue(expected_issue, issue);
    EXPECT_TRUE(difference.empty()) << difference;
  }
}

TEST_F(TestRightOfWayWithTrafficLights, ValidConfiguration)  // NOLINT for gtest
{
  load_target_map("intersection/right_of_way.osm");

  lanelet::autoware::validation::RightOfWayWithTrafficLightsValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0) << "right_of_way.osm should pass vm-03-10 validation";
}
