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

#ifndef LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LONGITUDINAL_SUBTYPE_CONNECTION_HPP_
#define LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LONGITUDINAL_SUBTYPE_CONNECTION_HPP_

#include <lanelet2_traffic_rules/GenericTrafficRules.h>
#include <lanelet2_validation/Validation.h>
#include <lanelet2_validation/ValidatorFactory.h>

#include <string>

namespace lanelet::autoware::validation
{
class LongitudinalSubtypeConnectionValidator : public lanelet::validation::MapValidator
{
public:
  // Write the validator's name here
  constexpr static const char * name() { return "mapping.lane.longitudinal_subtype_connection"; }

  lanelet::validation::Issues operator()(const lanelet::LaneletMap & map) override;

private:
  lanelet::validation::Issues check_longitudinal_subtype_connection(
    const lanelet::LaneletMap & map);
};

namespace traffic_rules
{
class LongitudinalSubtypeConnectionValidationRules
: public lanelet::traffic_rules::GenericTrafficRules
{
public:
  using GenericTrafficRules::GenericTrafficRules;

protected:
  Optional<bool> canPass(const RegulatoryElementConstPtrs & /*regElems*/) const override
  {
    return {};
  }
  Optional<bool> canPass(const std::string & type, const std::string & /*location*/) const override;

  const lanelet::traffic_rules::CountrySpeedLimits & countrySpeedLimits() const override
  {
    return speedLimits_;
  }
  Optional<lanelet::traffic_rules::SpeedLimitInformation> speedLimit(
    const RegulatoryElementConstPtrs & /*regelems*/) const override
  {
    return {};
  };

private:
  lanelet::traffic_rules::CountrySpeedLimits speedLimits_;
};

class LongitudinalSubtypeConnectionValidationPedestrianRules
: public lanelet::traffic_rules::GenericTrafficRules
{
public:
  using GenericTrafficRules::GenericTrafficRules;

protected:
  Optional<bool> canPass(const RegulatoryElementConstPtrs & /*regElems*/) const override
  {
    return {};
  }
  Optional<bool> canPass(const std::string & type, const std::string & /*location*/) const override;

  const lanelet::traffic_rules::CountrySpeedLimits & countrySpeedLimits() const override
  {
    return speedLimits_;
  }
  Optional<lanelet::traffic_rules::SpeedLimitInformation> speedLimit(
    const RegulatoryElementConstPtrs & /*regelems*/) const override
  {
    return {};
  };

private:
  lanelet::traffic_rules::CountrySpeedLimits speedLimits_;
};
}  // namespace traffic_rules
}  // namespace lanelet::autoware::validation

#endif  // LANELET2_MAP_VALIDATOR__VALIDATORS__LANE__LONGITUDINAL_SUBTYPE_CONNECTION_HPP_
