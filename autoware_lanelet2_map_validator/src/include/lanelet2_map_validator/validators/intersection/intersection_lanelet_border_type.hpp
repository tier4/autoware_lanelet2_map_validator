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

#ifndef AUTOWARE_LANELET2_MAP_VALIDATOR__TEMPLATE__VALIDATOR_TEMPLATE_HPP_
#define AUTOWARE_LANELET2_MAP_VALIDATOR__TEMPLATE__VALIDATOR_TEMPLATE_HPP_

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

namespace lanelet::autoware::validation
{
class IntersectionLaneletBorderTypeValidator : public lanelet::validation::MapValidator
{
public:
  constexpr static const char * name()
  {
    return "mapping.intersection.lanelet_border_type";
  }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
  lanelet::validation::Issues check_intersection_lanelet_border_type(const lanelet::LaneletMap & map);
};
}  // namespace lanelet::autoware::validation

#endif  // AUTOWARE_LANELET2_MAP_VALIDATOR__TEMPLATE__VALIDATOR_TEMPLATE_HPP_
