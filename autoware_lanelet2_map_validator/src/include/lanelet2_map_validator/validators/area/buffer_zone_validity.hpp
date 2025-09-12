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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__AREA__BUFFER_ZONE_VALIDITY_HPP_  // NOLINT
#define LANELET2_MAP_VALIDATOR__VALIDATORS__AREA__BUFFER_ZONE_VALIDITY_HPP_  // NOLINT

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

namespace lanelet::autoware::validation
{
class BufferZoneValidity : public lanelet::validation::MapValidator
{
public:
  constexpr static const char * name() { return "mapping.area.buffer_zone_validity"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
  lanelet::validation::Issues check_buffer_zone_validity(const lanelet::LaneletMap & map);
};
}  // namespace lanelet::autoware::validation

// clang-format off
#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__AREA__BUFFER_ZONE_VALIDITY_HPP_  // NOLINT
// clang-format on
