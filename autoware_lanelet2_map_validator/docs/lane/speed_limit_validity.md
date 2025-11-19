# speed_limit_validity

## Validator name

mapping.lane.speed_limit_validity

## Feature

Validates that lanelets with "road" or "private" subtype have a valid speed_limit attribute value. If a speed_limit attribute is present, it must be a positive numerical value.

| Issue Code                  | Message                                                                                                                                                     | Severity | Primitive | Description                                                                             | Approach                                                                         |
| --------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- | -------- | --------- | --------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| Lane.SpeedLimitValidity-001 | This {subtype} lanelet has an invalid speed_limit attribute value '{speed_limit_value}'. The value must be a positive numerical value.                      | Error    | lanelet   | The speed_limit attribute value is either not a valid number or is not positive (≤ 0.0) | Check if the speed_limit value can be parsed as a number and is greater than 0.0 |
| Lane.SpeedLimitValidity-002 | This {subtype} lanelet has a speed_limit attribute value '{speed_limit_value}' that is outside the configured range [{min_speed_limit}, {max_speed_limit}]. | Error    | lanelet   | The speed_limit attribute value is outside the configured min/max range                 | Check if the speed_limit value is within the configured range                    |

## Parameters

| Parameter Name  | Type   | Default | Description                        |
| --------------- | ------ | ------- | ---------------------------------- |
| max_speed_limit | double | 50.0    | Maximum allowed speed limit (km/h) |
| min_speed_limit | double | 10.0    | Minimum allowed speed limit (km/h) |

## Related source codes

- speed_limit_validity.cpp
- speed_limit_validity.hpp
