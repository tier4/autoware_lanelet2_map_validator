# speed_limit_validity

## Validator name

mapping.lane.speed_limit_validity

## Feature

Validates that lanelets with "road" or "private" subtype have a valid speed_limit attribute value. If a speed_limit attribute is present, it must be a positive numerical value.

| Issue Code                  | Message                                                                                                                                | Severity | Primitive | Description                                                                             | Approach                                                                         |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- | -------- | --------- | --------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| Lane.SpeedLimitValidity-001 | This {subtype} lanelet has an invalid speed_limit attribute value '{speed_limit_value}'. The value must be a positive numerical value. | Error    | lanelet   | The speed_limit attribute value is either not a valid number or is not positive (≤ 0.0) | Check if the speed_limit value can be parsed as a number and is greater than 0.0 |

## Parameters

None.

## Related source codes

- speed_limit_validity.cpp
- speed_limit_validity.hpp
