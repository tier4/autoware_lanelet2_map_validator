# intersection_area_tagging

## Validator name

mapping.intersection.intersection_area_tagging

## Feature

This validator ensures consistency between `intersection_area` polygons and lanelets with `intersection_area` tags. It validates that:

1. All lanelets completely covered by an `intersection_area` polygon have the correct `intersection_area` tag with the polygon's ID
2. All lanelets with an `intersection_area` tag are completely covered by the referenced intersection area polygon
3. All lanelets with a `turn_direction` tag also have an `intersection_area` tag

The validator outputs the following issues with the corresponding ID of the primitive.

| Issue Code                               | Message                                                                                                                     | Severity | Primitive | Description                                                                                              | Approach                                                                                                                    |
| ---------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- | -------- | --------- | -------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| Intersection.IntersectionAreaTagging-001 | "This lanelet is covered by intersection area {area_id} but missing intersection_area tag."                                 | Error    | Lanelet   | The lanelet is completely within an intersection area polygon but doesn't have the intersection_area tag | Add an `intersection_area` tag to the lanelet with the value set to the ID of the covering intersection area polygon        |
| Intersection.IntersectionAreaTagging-002 | "This lanelet is covered by intersection area {expected_area_id} but has incorrect intersection_area tag {actual_area_id}." | Error    | Lanelet   | The lanelet is covered by one intersection area but tagged with a different intersection area ID         | Update the `intersection_area` tag value to match the ID of the intersection area polygon that actually covers this lanelet |
| Intersection.IntersectionAreaTagging-003 | "This lanelet has intersection_area tag {area_id} but is not completely covered by that intersection area."                 | Error    | Lanelet   | The lanelet has an intersection_area tag but is not completely within the referenced intersection area   | Either remove the `intersection_area` tag or adjust the intersection area polygon to completely cover the lanelet           |
| Intersection.IntersectionAreaTagging-004 | "This lanelet has turn_direction tag '{turn_direction}' but is missing intersection_area tag."                              | Error    | Lanelet   | The lanelet has a turn_direction tag but is missing the required intersection_area tag                   | Add an `intersection_area` tag to the lanelet with the appropriate intersection area ID                                     |

## Parameters

None.

## Related source codes

- intersection_area_tagging.cpp
- intersection_area_tagging.hpp
