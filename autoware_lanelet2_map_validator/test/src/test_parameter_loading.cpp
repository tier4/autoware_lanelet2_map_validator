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

#include "lanelet2_map_validator/parameter.hpp"
#include "lanelet2_map_validator/validators/traffic_light/missing_referrers_for_traffic_lights.hpp"
#include "map_validation_tester.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>

#include <gtest/gtest.h>
#include <lanelet2_validation/Validation.h>

#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace lanelet::autoware::validation
{
class ParameterLoadingTest : public MapValidationTester
{
protected:
};

TEST_F(ParameterLoadingTest, ParseWithGetParameter)
{
  const std::string package_share_directory =
    ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");
  const std::string full_path = package_share_directory + "/data/test_params.yaml";
  ParameterLoader loader_foo("mapping.foo.foo", full_path);
  ParameterLoader loader_bar("mapping.bar.bar", full_path);
  ParameterLoader loader_baz("mapping.baz.baz", full_path);

  EXPECT_EQ(loader_foo.get_parameter<double>("AAA").value(), 5.0);
  EXPECT_EQ(loader_foo.get_parameter<int>("BBB").value(), 1234);
  EXPECT_EQ(loader_foo.get_parameter<int>("CCC"), std::nullopt);

  EXPECT_EQ(loader_bar.get_parameter<std::string>("DDD").value(), "Apple");
  EXPECT_EQ(loader_bar.get_parameter<std::string>("EEE").value(), "banana");
  EXPECT_EQ(loader_bar.get_parameter<std::string>("FFF").value(), "3.14");

  EXPECT_EQ(loader_baz.get_parameter<bool>("GGG").value(), true);
  EXPECT_EQ(loader_baz.get_parameter<bool>("FFF").value(), false);
  EXPECT_EQ(loader_baz.get_parameter<bool>("HHH").value(), true);
  EXPECT_EQ(loader_baz.get_parameter<bool>("III"), std::nullopt);
}

TEST_F(ParameterLoadingTest, ParseWithGetParameterOr)
{
  const std::string package_share_directory =
    ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");
  const std::string full_path = package_share_directory + "/data/test_params.yaml";
  ParameterLoader loader_foo("mapping.foo.foo", full_path);
  ParameterLoader loader_bar("mapping.bar.bar", full_path);
  ParameterLoader loader_baz("mapping.baz.baz", full_path);

  EXPECT_EQ(loader_foo.get_parameter_or<double>("AAA", 3.0), 5.0);
  EXPECT_EQ(loader_foo.get_parameter_or<int>("BBB", 4321), 1234);
  EXPECT_EQ(loader_foo.get_parameter_or<int>("CCC", 5), 5);

  EXPECT_EQ(loader_bar.get_parameter_or<std::string>("DDD", "apple"), "Apple");
  EXPECT_EQ(loader_bar.get_parameter_or<std::string>("EEE", "BANANA"), "banana");
  EXPECT_EQ(loader_bar.get_parameter_or<std::string>("FFF", "pi"), "3.14");

  EXPECT_EQ(loader_baz.get_parameter_or<bool>("GGG", false), true);
  EXPECT_EQ(loader_baz.get_parameter_or<bool>("FFF", true), false);
  EXPECT_EQ(loader_baz.get_parameter_or<bool>("HHH", false), true);
  EXPECT_EQ(loader_bar.get_parameter_or<bool>("III", true), true);
}

TEST_F(ParameterLoadingTest, CheckDefaultParamsFileValidity)
{
  load_target_map("sample_map.osm");  // needed to trigger validator registration
  lanelet::validation::Strings validators =
    lanelet::validation::availabeChecks("");  // cspell:disable-line

  const std::string package_share_directory =
    ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");
  const std::string full_path = package_share_directory + "/config/params.yaml";
  YAML::Node root = YAML::LoadFile(full_path);

  EXPECT_TRUE(root.IsMap());

  for (const auto & child : root) {
    std::string child_name = child.first.as<std::string>();
    EXPECT_TRUE(std::find(validators.begin(), validators.end(), child_name) != validators.end());
    EXPECT_TRUE(child.second.IsMap());

    bool has_grandchildren = false;
    for (const auto & subitem : child.second) {
      if (!subitem.second.IsScalar()) {
        has_grandchildren = true;
        break;
      }
    }
    EXPECT_FALSE(has_grandchildren);
  }
}

}  // namespace lanelet::autoware::validation
