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

#include "lanelet2_map_validator/cli.hpp"
#include "lanelet2_map_validator/config_store.hpp"
#include "lanelet2_map_validator/io.hpp"
#include "lanelet2_map_validator/map_loader.hpp"
#include "lanelet2_map_validator/utils.hpp"
#include "lanelet2_map_validator/validation.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char * argv[])
{
  lanelet::autoware::validation::MetaConfig meta_config =
    lanelet::autoware::validation::parseCommandLine(
      argc, const_cast<const char **>(argv));  // NOLINT

  (void)meta_config;

  // Print help (Already done in parseCommandLine)
  if (meta_config.command_line_config.help) {
    return 0;
  }

  // Print available validators
  if (meta_config.command_line_config.print) {
    auto checks = lanelet::validation::availabeChecks(  // cspell:disable-line
      meta_config.command_line_config.validationConfig.checksFilter);
    if (checks.empty()) {
      std::cout << "No checks found matching to '"
                << meta_config.command_line_config.validationConfig.checksFilter << "'"
                << std::endl;
    } else {
      std::cout << "The following checks are available:" << std::endl;
      for (auto & check : checks) {
        std::cout << check << std::endl;
      }
    }
    return 0;
  }

  // Check map file
  if (meta_config.command_line_config.mapFile.empty()) {
    throw std::invalid_argument("No map file specified!");
  }
  if (!std::filesystem::is_regular_file(meta_config.command_line_config.mapFile)) {
    throw std::invalid_argument("Map file doesn't exist or is not a file!");
  }

  // Load map and catch loading_issues
  const auto [lanelet_map_ptr, loading_issues] = lanelet::autoware::validation::loadAndValidateMap(
    meta_config.projector_type, meta_config.command_line_config.mapFile,
    meta_config.command_line_config.validationConfig);

  if (!loading_issues[0].issues.empty()) {
    std::cout << "Errors found on map loading." << std::endl;
    lanelet::validation::printAllIssues(loading_issues);
  }

  // Load exclusion list
  lanelet::autoware::validation::ValidatorExclusionMap exclusion_map;
  if (!meta_config.exclusion_list.empty()) {
    if (!std::filesystem::is_regular_file(meta_config.exclusion_list)) {
      throw std::invalid_argument("Exclusion list doesn't exist or is not a file!");
    }
    std::ifstream exclusion_list(meta_config.exclusion_list);
    json exclusion_list_json;
    exclusion_list >> exclusion_list_json;

    exclusion_map = lanelet::autoware::validation::import_exclusion_list(exclusion_list_json);
  } else {
    for (const std::string & validator_name :
         lanelet::validation::availabeChecks(".*")) {  // cspell:disable-line
      exclusion_map[validator_name] = std::vector<lanelet::autoware::validation::SimplePrimitive>();
    }
  }

  // Load parameters and issues_info files
  std::string package_share_directory =
    ament_index_cpp::get_package_share_directory("autoware_lanelet2_map_validator");
  std::string parameters_file = (!meta_config.parameters_file.empty())
                                  ? meta_config.parameters_file
                                  : package_share_directory + "/config/params.yaml";
  std::string issues_info_file =
    package_share_directory +
    "/config/issues_info.json";  // We think issues_info should NOT be derived for now
  lanelet::autoware::validation::ValidatorConfigStore::initialize(
    parameters_file, issues_info_file, meta_config.language);

  // Validation against lanelet::LaneletMap object
  if (!lanelet_map_ptr) {
    throw std::invalid_argument("The map file was not possible to load!");
  } else if (!meta_config.requirements_file.empty()) {
    if (!std::filesystem::is_regular_file(meta_config.requirements_file)) {
      throw std::invalid_argument("Input JSON file doesn't exist or is not a file!");
    }
    std::ifstream input_file(meta_config.requirements_file);
    json json_data;
    input_file >> json_data;

    const auto mapping_issues = lanelet::autoware::validation::validate_all_requirements(
      json_data, meta_config, *lanelet_map_ptr, exclusion_map);

    lanelet::autoware::validation::summarize_validator_results(json_data);
    lanelet::validation::printAllIssues(mapping_issues);

    lanelet::autoware::validation::insert_validator_info_to_map(
      meta_config.command_line_config.mapFile,
      std::filesystem::path(meta_config.requirements_file).filename().string(),
      json_data.value("version", ""));

    if (!meta_config.output_file_path.empty()) {
      lanelet::autoware::validation::insert_validation_info_to_json(json_data, meta_config);
      lanelet::autoware::validation::export_results(json_data, meta_config.output_file_path);
    }
  } else {
    auto issues = lanelet::autoware::validation::apply_validation(
      *lanelet_map_ptr, meta_config.command_line_config.validationConfig);
    for (const std::string & validator_name :
         lanelet::validation::availabeChecks(".*")) {  // cspell:disable-line
      lanelet::autoware::validation::filter_out_primitives(
        issues, exclusion_map.at(validator_name));
    }
    lanelet::validation::printAllIssues(issues);
  }

  return 0;
}
