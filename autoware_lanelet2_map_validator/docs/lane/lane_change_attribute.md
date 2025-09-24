# lane_change_attribute

## Validator name

mapping.lane.lane_change_attribute

## Feature

This validator checks that linestrings shared between adjacent road-type lanelets have the required `lane_change` attribute and that its value is valid. The `lane_change` attribute indicates whether lane changing is allowed across that boundary and must be set to either "yes" or "no".

The validator implements the VM-01-02 requirement by:
- Identifying linestrings that are shared between two adjacent, same-direction, road-type lanelets
- Checking that each shared linestring has the `lane_change` attribute
- Validating that the attribute value is either "yes" or "no"

| Issue Code                 | Message                                                                                                                          | Severity | Primitive  | Description                                                                                                                   | Approach                                                                                              |
| -------------------------- | -------------------------------------------------------------------------------------------------------------------------------- | -------- | ---------- | ----------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| Lane.LaneChangeAttribute-001 | This {bound_type} bound linestring of a road type lanelet is missing a lane_change attribute.                                   | Error    | linestring | A linestring shared between adjacent road lanelets is missing the required `lane_change` attribute.                          | Add a `lane_change` attribute with value "yes" or "no" to the linestring.                           |
| Lane.LaneChangeAttribute-002 | This {bound_type} bound linestring has an invalid lane_change attribute value '{invalid_value}'. Valid values are 'yes' or 'no'. | Error    | linestring | A linestring has a `lane_change` attribute but the value is not "yes" or "no".                                               | Change the `lane_change` attribute value to either "yes" or "no".                                   |

## Parameters

None.

## Related source codes

- lane_change_attribute.cpp
- lane_change_attribute.hpp
