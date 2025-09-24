# road_lanelet_attribute

## Validator name

mapping.lane.road_lanelet_attribute

## Feature

This validator implements the VM-01-01 requirements for road type lanelets as specified in the Autoware vector map requirements. It ensures that all road type lanelets have the required attributes properly set.

**VM-01-01 Requirements:**

- All road type lanelets must have the `location` attribute set to "urban"
- All road type lanelets must have the `one_way` attribute set to "yes"

The validator checks all lanelets in the map and validates only those with `subtype="road"`.

| Issue Code                    | Message                                                                                | Severity | Primitive | Description                                        | Approach                                                         |
| ----------------------------- | -------------------------------------------------------------------------------------- | -------- | --------- | -------------------------------------------------- | ---------------------------------------------------------------- |
| Lane.RoadLaneletAttribute-001 | Road lanelet is missing the required 'location' attribute or it is not set to 'urban'. | Error    | lanelet   | Validates that road lanelets have location="urban" | Check if lanelet has location attribute and its value is "urban" |
| Lane.RoadLaneletAttribute-002 | Road lanelet is missing the required 'one_way' attribute or it is not set to 'yes'.    | Error    | lanelet   | Validates that road lanelets have one_way="yes"    | Check if lanelet has one_way attribute and its value is "yes"    |

## Parameters

None.

## Related source codes

- road_lanelet_attribute.cpp
- road_lanelet_attribute.hpp
