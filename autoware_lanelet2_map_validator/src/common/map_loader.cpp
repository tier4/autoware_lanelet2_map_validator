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

#include "lanelet2_map_validator/map_loader.hpp"

#include <GeographicLib/MGRS.hpp>
#include <GeographicLib/UTMUPS.hpp>
#include <autoware_lanelet2_extension/projection/mgrs_projector.hpp>
#include <autoware_lanelet2_extension/projection/transverse_mercator_projector.hpp>
#include <pugixml.hpp>

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace lanelet::autoware::validation
{

namespace
{
namespace projector_names
{
constexpr const char * mgrs = "mgrs";
constexpr const char * transverse_mercator = "transverse_mercator";
constexpr const char * utm = "utm";
}  // namespace projector_names

constexpr int mgrs_100km_grid_precision = 0;

std::vector<lanelet::GPSPoint> load_gps_points(const std::string & map_file)
{
  std::vector<lanelet::GPSPoint> points;
  pugi::xml_document doc;
  if (!doc.load_file(map_file.c_str())) {
    return points;
  }
  for (const pugi::xml_node & node : doc.child("osm").children("node")) {
    const pugi::xml_attribute lat = node.attribute("lat");
    const pugi::xml_attribute lon = node.attribute("lon");
    if (lat && lon) {
      points.push_back({lat.as_double(), lon.as_double()});
    }
  }
  return points;
}

std::string mgrs_cross_grid_error(const std::set<std::string> & grids)
{
  std::string grid_list;
  for (const auto & grid : grids) {
    grid_list += (grid_list.empty() ? "" : ", ") + grid;
  }
  return "The map spans multiple MGRS grids (" + grid_list +
         ") and cannot be projected correctly by the mgrs projector. "
         "Use another projector such as transverse_mercator.";
}
}  // namespace

std::set<std::string> get_mgrs_grids(const std::vector<lanelet::GPSPoint> & points)
{
  std::set<std::string> grids;
  for (const auto & point : points) {
    int zone{};
    bool is_north{};
    double utm_x{};
    double utm_y{};
    std::string mgrs_code;
    try {
      GeographicLib::UTMUPS::Forward(point.lat, point.lon, zone, is_north, utm_x, utm_y);
      GeographicLib::MGRS::Forward(
        zone, is_north, utm_x, utm_y, point.lat, mgrs_100km_grid_precision, mgrs_code);
    } catch (const GeographicLib::GeographicErr &) {
      continue;
    }
    grids.insert(mgrs_code);
  }
  return grids;
}

bool is_in_same_mgrs_grid(const std::vector<lanelet::GPSPoint> & points)
{
  return get_mgrs_grids(points).size() <= 1;
}

std::unique_ptr<lanelet::Projector> getProjector(
  const std::string & projector_type, const lanelet::GPSPoint & origin)

{
  if (projector_type == projector_names::mgrs) {
    return std::make_unique<lanelet::projection::MGRSProjector>();
  }
  if (projector_type == projector_names::transverse_mercator) {
    return std::make_unique<lanelet::projection::TransverseMercatorProjector>(
      lanelet::Origin{origin});
  }
  if (projector_type == projector_names::utm) {
    return std::make_unique<lanelet::projection::UtmProjector>(lanelet::Origin{origin});
  }
  return nullptr;
}

std::pair<lanelet::LaneletMapPtr, std::vector<lanelet::validation::DetectedIssues>>
loadAndValidateMap(
  const std::string & projector_type, const std::string & map_file,
  const lanelet::validation::ValidationConfig & val_config)
{
  std::vector<lanelet::validation::DetectedIssues> issues;
  lanelet::LaneletMapPtr map{nullptr};
  lanelet::validation::Strings errors;
  try {
    const auto projector = getProjector(projector_type, val_config.origin);
    if (!projector) {
      errors.push_back("No valid map projection type specified!");
    } else {
      map = lanelet::load(map_file, *projector, &errors);
    }
    if (!map) {
      errors.push_back("Failed to load map!");
    }
    if (map && projector_type == projector_names::mgrs) {
      const auto points = load_gps_points(map_file);
      if (!is_in_same_mgrs_grid(points)) {
        errors.push_back(mgrs_cross_grid_error(get_mgrs_grids(points)));
      }
    }
    issues.emplace_back("general", utils::transform(errors, [](auto & error) {
                          return lanelet::validation::Issue(
                            lanelet::validation::Severity::Error, error);
                        }));
  } catch (lanelet::LaneletError & err) {
    issues.emplace_back("general", utils::transform(errors, [](auto & error) {
                          return lanelet::validation::Issue(
                            lanelet::validation::Severity::Error, error);
                        }));
  }

  return {map, issues};
}

}  // namespace lanelet::autoware::validation
