# longitudinal_subtype_connection

## Validator name

mapping.lane.longitudinal_subtype_connection

## Feature

This validator checks whether the connection of two lanelets in the driving direction (forwards and backwards) are not breaking the following rules.

- `crosswalk` or `walkway` subtype lanelets should not have a successor lanelet
- Two connected lanelets should have the same subtype.

The validator outputs the following issue with the corresponding ID of the primitive.

| Issue Code                             | Message                                                              | Severity | Primitive | Description                                                                | Approach                                                                         |
| -------------------------------------- | -------------------------------------------------------------------- | -------- | --------- | -------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| Lane.LongitudinalSubtypeConnection-001 | A crosswalk or walkway type lanelet cannot have a successor lanelet. | Error    | Lanelet   | The crosswalk or walkway lanelet has a successor lanelet but it shouldn't. | Concatenate those crosswalk or walkway lanelets to one.                          |
| Lane.LongitudinalSubtypeConnection-002 | A lanelet and its successor must have the same subtype.              | Error    | Lanelet   | The next lanelet of this lanelet has a different subtype.                  | Make sure that lanelets connected in the driving direction has the same subtype. |

## Related source codes

- longitudinal_subtype_connection.cpp
- longitudinal_subtype_connection.hpp
