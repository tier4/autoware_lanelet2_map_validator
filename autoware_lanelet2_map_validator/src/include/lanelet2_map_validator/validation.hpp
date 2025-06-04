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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATION_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATION_HPP_

#include "lanelet2_map_validator/cli.hpp"
#include "lanelet2_map_validator/utils.hpp"

#include <nlohmann/json.hpp>

#include <lanelet2_validation/Cli.h>
#include <lanelet2_validation/Validation.h>

#include <map>
#include <queue>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using json = nlohmann::json;

namespace lanelet::autoware::validation
{

/**
 * @brief alias for code clarity
 */
using ValidatorName = std::string;

struct ValidatorInfo
{
  enum class Severity { ERROR, WARNING, INFO, NONE };

  std::unordered_map<ValidatorName, bool>
    prereq_with_forgive_warnings;  //<! Validator(1st), forgive_warning(2nd)

  Severity max_severity = Severity::NONE;
};

using Validators = std::unordered_map<ValidatorName, ValidatorInfo>;

/**
 * @brief 1st is either of "point", "linestring", "polygon", "lanelet", "area", "regulatory
 * element", "primitive"
 */
using SimplePrimitive = std::pair<std::string, lanelet::Id>;

using ValidatorExclusionMap = std::map<ValidatorName, std::vector<SimplePrimitive>>;

/**
 * @brief simply call lanelet::validation::validateMap
 * @return return lanelet::validation::validateMap()
 */
std::vector<lanelet::validation::DetectedIssues> apply_validation(
  const lanelet::LaneletMap & lanelet_map,
  const lanelet::validation::ValidationConfig & val_config);

Validators parse_validators(const json & json_data);

/**
 * @brief topologically sort the validators
 * @return names of validators(1st), remaining validators(2nd, usually normal)
 */
std::tuple<std::queue<ValidatorName>, Validators> create_validation_queue(
  const Validators & validators);

/**
 * @brief find a validator block by name
 */
json & find_validator_block(json & json_data, const ValidatorName & validator_name);

/**
 * @brief set error to json and output it for invalid prerequisite setting
 */
std::vector<lanelet::validation::DetectedIssues> describe_unused_validators_to_json(
  json & json_data, const Validators & unused_validators);

/**
 * @brief if the prerequisites are SEVERE or non-ignorable WARNING, report that "Prerequisites
 * didn't pass"
 */
std::vector<lanelet::validation::DetectedIssues> check_prerequisite_completion(
  const Validators & validators, const ValidatorName & target_validator_name);

/**
 * @brief check if requirement have passed, count the number of error/warning, etc., then set it to
 * json_data and print to stdout
 */
void summarize_validator_results(json & json_data);

/**
 * @brief helper to create a ValidationConfig with .checksFilter = {validator_name}
 */
lanelet::validation::ValidationConfig replace_validator(
  const lanelet::validation::ValidationConfig & input, const ValidatorName & validator_name);

std::vector<lanelet::validation::DetectedIssues> validate_all_requirements(
  json & json_data, const lanelet::autoware::validation::MetaConfig & validator_config,
  const lanelet::LaneletMap & lanelet_map, const ValidatorExclusionMap & exclusion_map);

void export_results(json & json_data, const std::string output_file_path);

ValidatorExclusionMap import_exclusion_list(const json & json_data);

/**
 * @brief this functions is used by giving ValidationExclusionMap[validator_foo] as the second
 * argument and filter out the 1st argument
 */
void filter_out_primitives(
  std::vector<lanelet::validation::DetectedIssues> & issues_vector,
  std::vector<SimplePrimitive> primitive_list);
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATION_HPP_
