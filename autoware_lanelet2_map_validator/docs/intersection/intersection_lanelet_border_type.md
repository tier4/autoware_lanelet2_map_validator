# intersection_lanelet_border_type

## Validator name

mapping.intersection.lanelet_border_type

## Feature

This validator checks whether lanelets that have an `intersection_area` tag have proper border types. Specifically, it validates that both the LeftBound and RightBound of such lanelets are either of type "virtual" or "road_border".

The validator will output the following issues with the corresponding primitive ID.

| Issue Code                         | Message                                                                                                                 | Severity | Primitive | Description                                                                                                                                        | Approach                                                                                                                                         |
| ---------------------------------- | ----------------------------------------------------------------------------------------------------------------------- | -------- | --------- | -------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| Intersection.VirtualBorderType-001 | The LeftBound and RightBound of this lanelet with intersection_area tag must be either "virtual" or "road_border" type. | Error    | Lanelet   | Lanelets with intersection_area tags must have their borders properly typed as either "virtual" or "road_border" for proper intersection handling. | Set the type attribute of the LeftBound and RightBound linestrings to either "virtual" or "road_border" depending on the nature of the boundary. |

## Parameters

None.

## Related source codes

- intersection_lanelet_border_type.cpp
- intersection_lanelet_border_type.hpp
