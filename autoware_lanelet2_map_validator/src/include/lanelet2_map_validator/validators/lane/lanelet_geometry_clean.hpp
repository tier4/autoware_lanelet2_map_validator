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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANELET_GEOMETRY_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANELET_GEOMETRY_HPP_

#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>
#include <memory>

namespace lanelet::autoware::validation
{
class LaneletGeometryValidator : public lanelet::validation::MapValidator
{
public:
    constexpr static const char * name() { return "mapping.lane.lanelet_geometry"; }

    lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
    class IsolationForestPredictor;
    lanelet::validation::Issues check_lanelet_geometry(const lanelet::LaneletMap & map);
    lanelet::validation::Issues check_lanelet_anomaly_if(
    const lanelet::LaneletMap & map,
    const std::string & model_path,
    double anomaly_threshold = 0.6);

    mutable std::unique_ptr<IsolationForestPredictor> if_model_ = nullptr;
    mutable bool if_model_loaded_ = false;
};
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LANELET_GEOMETRY_HPP_
