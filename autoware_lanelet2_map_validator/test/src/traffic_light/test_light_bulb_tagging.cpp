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

#include "lanelet2_map_validator/validators/traffic_light/light_bulb_tagging.hpp"
#include "map_validation_tester.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/LaneletMap.h>

#include <string>

class TestLightBulbsTaggingValidator : public MapValidationTester
{
private:
};

TEST_F(TestLightBulbsTaggingValidator, ValidatorAvailability)  // NOLINT for gtest
{
  std::string expected_validator_name =
    lanelet::autoware::validation::LightBulbsTaggingValidator::name();

  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks(expected_validator_name);  // cspell:disable-line

  const uint32_t expected_validator_num = 1;
  EXPECT_EQ(expected_validator_num, validators.size());
  EXPECT_EQ(expected_validator_name, validators[0]);
}

TEST_F(TestLightBulbsTaggingValidator, MissingColor)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_colorless_arrow_bulb.osm");

  lanelet::autoware::validation::LightBulbsTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 9298);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::Point);
  EXPECT_EQ(
    issues[0].message,
    "[TrafficLight.LightBulbTagging-001] A point representing a light bulb should have a color "
    "tag.");
}

TEST_F(TestLightBulbsTaggingValidator, InvalidColor)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_invalid_color.osm");

  lanelet::autoware::validation::LightBulbsTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 8792);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::Point);
  EXPECT_EQ(
    issues[0].message,
    "[TrafficLight.LightBulbTagging-002] The color of a light bulb should be either red, yellow, "
    "or green.");
}

TEST_F(TestLightBulbsTaggingValidator, InvalidArrowDirection)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_invalid_arrow_direction.osm");

  lanelet::autoware::validation::LightBulbsTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 1);
  EXPECT_EQ(issues[0].id, 9298);
  EXPECT_EQ(issues[0].severity, lanelet::validation::Severity::Error);
  EXPECT_EQ(issues[0].primitive, lanelet::validation::Primitive::Point);
  EXPECT_EQ(
    issues[0].message,
    "[TrafficLight.LightBulbTagging-003] The arrow of a light bulb should be either up, right, "
    "left, up_right, or up_left.");
}

TEST_F(TestLightBulbsTaggingValidator, CorrectTrafficLight)  // NOLINT for gtest
{
  load_target_map("traffic_light/traffic_light_with_arrow_bulb.osm");

  lanelet::autoware::validation::LightBulbsTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}

TEST_F(TestLightBulbsTaggingValidator, SampleMap)  // NOLINT for gtest
{
  load_target_map("sample_map.osm");

  lanelet::autoware::validation::LightBulbsTaggingValidator checker;
  const auto & issues = checker(*map_);

  EXPECT_EQ(issues.size(), 0);
}
