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

#include "lanelet2_map_validator/validation.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{

class PrimitiveExclusionTest : public ::testing::Test
{
protected:
};

TEST_F(PrimitiveExclusionTest, GenerateValidatorExclusionMap)  // NOLINT for gtest
{
  const std::string package_share_directory =
    ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");

  const std::string test_json_file_name =
    package_share_directory + "/data/json/test_primitive_exclusion.json";

  std::ifstream test_json_file(test_json_file_name);
  json exclusion_list;
  test_json_file >> exclusion_list;

  const std::vector<SimplePrimitive> expected_primitives = {
    {"point", 1}, {"linestring", 2},         {"polygon", 3},  {"lanelet", 4},
    {"area", 5},  {"regulatory element", 6}, {"primitive", 7}};

  const std::vector<std::string> validator_names =
    lanelet::validation::availabeChecks(".*");  // cspell:disable-line

  const ValidatorExclusionMap exclusion_map = import_exclusion_list(exclusion_list);

  ASSERT_EQ(exclusion_map.size(), validator_names.size());
  ASSERT_GT(
    exclusion_map.size(), 8);  // greater then the installed validators from lanelet2_validation

  for (const auto & [validator, list] : exclusion_map) {
    for (const auto & primitive : expected_primitives) {
      if (primitive.second == 2) {
        if (
          validator == "mapping.traffic_light.missing_regulatory_elements" ||
          validator == "mapping.traffic_light.correct_facing") {
          EXPECT_TRUE(std::find(list.begin(), list.end(), primitive) != list.end());
        } else {
          EXPECT_TRUE(std::find(list.begin(), list.end(), primitive) == list.end());
        }
      } else {
        EXPECT_TRUE(std::find(list.begin(), list.end(), primitive) != list.end());
      }
    }
  }
}

TEST_F(PrimitiveExclusionTest, FilterOutPrimitives)  // NOLINT for gtest
{
  lanelet::validation::Issues test_issues;

  for (int i = 1; i <= 4; i++) {
    test_issues.push_back(
      {lanelet::validation::Severity::Error, lanelet::validation::Primitive::Lanelet, i,
       "dummy message"});
  }

  std::vector<lanelet::validation::DetectedIssues> detected_issues_vector;
  detected_issues_vector.push_back({"dummy_validator", test_issues});

  ValidatorExclusionMap exclusion_map;
  exclusion_map["dummy_validator"] = std::vector<SimplePrimitive>();
  exclusion_map.at("dummy_validator").push_back({"lanelet", 2});

  filter_out_primitives(detected_issues_vector, exclusion_map.at("dummy_validator"));

  EXPECT_EQ(detected_issues_vector[0].issues.size(), 3);

  for (const auto & issue : detected_issues_vector[0].issues) {
    EXPECT_NE(issue.id, 2);
  }
}

}  // namespace lanelet::autoware::validation
