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
#include <regex>
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

protected:
  std::set<std::string> extract_placeholders(const std::string & message);
  inline static const std::set<std::string> expected_severities_ = {"Error", "Warning", "info"};
  inline static const std::set<std::string> expected_primitives_ = {
    "point", "linestring", "polygon", "lanelet", "area", "regulatory element", "primitive"};
  std::set<std::string> expected_prefixes_ = {};
};

std::set<std::string> IssuesInfoTest::extract_placeholders(const std::string & message)
{
  std::set<std::string> result;
  std::regex placeholder_re(R"(\{([^}]+)\})");
  std::smatch match;

  std::string::const_iterator searchStart(message.cbegin());
  while (std::regex_search(searchStart, message.cend(), match, placeholder_re)) {
    result.insert(match[1].str());  // capture what's inside the braces
    searchStart = match.suffix().first;
  }

  return result;
}

TEST_F(IssuesInfoTest, CheckIssuesInfoIsValid)
{
  const auto issues_info = ValidatorConfigStore::issues_info();

  for (const auto & [title, block] : issues_info.items()) {
    const std::string prefix = title.substr(0, title.size() - 3);
    const std::string suffix = title.substr(title.size() - 3);

    // Does the corresponding validator exist
    EXPECT_TRUE(expected_prefixes_.count(prefix))
      << "Cannot find the issue code " << title << " that matches the existing validators!";

    // Is the severity valid
    ASSERT_TRUE(block.contains("severity")) << "Missing 'severity' in block: " << title;
    std::string severity = block["severity"];
    EXPECT_TRUE(expected_severities_.count(severity))
      << "Invalid severity: " << severity << " in block " << title;

    // Is the primitive valid
    ASSERT_TRUE(block.contains("primitive")) << "Missing 'primitive' in block: " << title;
    std::string primitive = block["primitive"];
    EXPECT_TRUE(expected_primitives_.count(primitive))
      << "Invalid primitive: " << primitive << " in block " << title;

    // Is the message block valid
    ASSERT_TRUE(block.contains("message")) << "Missing 'message' in block: " << title;
    ASSERT_TRUE(block["message"].is_object()) << "'message' must be a JSON block (" << title << ")";
    ASSERT_TRUE(block["message"].contains("en")) << "'message' must contain 'en' (" << title << ")";
    ASSERT_TRUE(block["message"]["en"].is_string())
      << "'message' must be a string (" << title << ")";

    const std::set<std::string> expected_placeholders =
      extract_placeholders(block["message"]["en"]);

    for (const auto & [lang, message_text] : block["message"].items()) {
      if (lang == "en") {
        continue;
      }
      ASSERT_TRUE(message_text.is_string()) << "'message' must be a string (" << title << ")";
      const std::set<std::string> placeholders = extract_placeholders(message_text);
      EXPECT_EQ(placeholders, expected_placeholders) << "'message's must have same substitutions";
    }
  }
}

}  // namespace lanelet::autoware::validation
