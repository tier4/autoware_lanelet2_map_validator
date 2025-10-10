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

#include "lanelet2_map_validator/io.hpp"

#include "lanelet2_map_validator/cli.hpp"
#include "lanelet2_map_validator/embedded_defaults.hpp"

#include <nlohmann/json.hpp>
#include <pugixml.hpp>

#include <filesystem>
#include <iostream>
#include <string>

namespace lanelet::autoware::validation
{
std::string get_validator_version()
{
  return package_version_str_;
}

void insert_validator_info_to_map(
  std::string osm_file, std::string requirements, std::string requirements_version)
{
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(osm_file.c_str());

  if (!result) {
    throw std::invalid_argument("Failed to load osm file!");
  }

  pugi::xml_node osm_node = doc.child("osm");
  if (!osm_node) {
    throw std::invalid_argument("No <osm> tag found in the osm file!");
  }

  pugi::xml_node validation_node = osm_node.child("validation");

  if (!validation_node) {
    validation_node = osm_node.prepend_child("validation");
  }

  // Helper function to set or append an attribute
  auto set_or_append_attribute = [](pugi::xml_node & node, const char * name, const char * value) {
    pugi::xml_attribute attr = node.attribute(name);
    if (attr) {
      attr.set_value(value);
    } else {
      node.append_attribute(name) = value;
    }
  };

  // Set or append the required attributes
  set_or_append_attribute(validation_node, "name", "autoware_lanelet2_map_validator");
  set_or_append_attribute(validation_node, "validator_version", get_validator_version().c_str());
  set_or_append_attribute(validation_node, "requirements", requirements.c_str());
  set_or_append_attribute(validation_node, "requirements_version", requirements_version.c_str());

  if (!doc.save_file(osm_file.c_str())) {
    throw std::runtime_error("Failed to save the validator info to osm file");
  }

  std::cout << "Modified validator information in the osm file." << std::endl;
}

void insert_validation_info_to_json(nlohmann::json & json_data, MetaConfig config)
{
  const std::filesystem::path path(config.command_line_config.mapFile);
  const std::string map_file_name =
    path.parent_path().filename().string() + "/" + path.filename().string();
  const std::string requirements_file_name =
    std::filesystem::path(config.requirements_file).filename().string();

  json_data["validation_info"] = {
    {"target_map", map_file_name},
    {"map_requirements",
     {{"filename", requirements_file_name}, {"version", json_data.value("version", "")}}},
    {"validator",
     {{"name", "autoware_lanelet2_map_validator"}, {"version", get_validator_version()}}}};
}
}  // namespace lanelet::autoware::validation
