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

#include "map_validation_tester.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace lanelet::autoware::validation
{

class IssuesInfoTest : public MapValidationTester
{
public:
  IssuesInfoTest()
  {
    load_target_map("sample_map.osm");
    lanelet::validation::Strings validators =
      lanelet::validation::availabeChecks("");  // cspell:disable-line

    for (const auto & val : validators) {
      std::string str = issue_code(val, 0);
      str.resize(str.size() - 3);  // remove "000"
      expected_prefixes_.insert(str);
    }
  }

private:
  inline static const std::set<std::string> expected_severities_ = {"Error", "Warning", "info"};
  inline static const std::set<std::string> expected_primitives_ = {
    "point", "linestring", "polygon", "lanelet", "area", "regulatory element", "primitive"};
  std::set<std::string> expected_prefixes_ = {};
};

TEST_F(IssuesInfoTest, CheckIssuesInfoIsValid)
{
  const auto issues_info = ValidatorConfigStore::issues_info();
}

}  // namespace lanelet::autoware::validation
