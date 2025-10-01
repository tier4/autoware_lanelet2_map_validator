# lanelet_division

## Validator name

mapping.intersection.lanelet_division

## Feature

This validator checks lanelets that have an intersection_area tag to ensure they don't have a successor with the same intersection_area tag value. This prevents improper lanelet division within intersection areas.

| Issue Code                       | Message                                                                                                                                                                                                         | Severity | Primitive | Description                                                                                                                                                               | Approach                                                                                                                         |
| -------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------- | --------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| Intersection.LaneletDivision-001 | Create the lanelets in the intersection as a single object without dividing them. This lanelet and its successor {successor_id} should be merged into one lanelet for intersection_area {intersection_area_id}. | Error    | lanelet   | A lanelet with an intersection_area tag has a successor that also has the same intersection_area tag value, indicating improper lanelet division within the intersection. | Merge the divided lanelets into a single lanelet object to properly represent the continuous path through the intersection area. |

## Parameters

None.

## Related source codes

- lanelet_division.cpp
- lanelet_division.hpp
