# lanelet_geometry

## Validator name

mapping.lane.lanelet_geometry

## Feature

This validator checks the geometric properties of lanelets. It ensures that the left bound and right bound of each lanelet do not share any points, which is a fundamental requirement for lanelet geometry validity.

| Issue Code               | Message                                                       | Severity | Primitive | Description                                                                                                                            | Approach                                                                                                     |
| ------------------------ | ------------------------------------------------------------- | -------- | --------- | -------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| Lane.LaneletGeometry-001 | This lanelet has shared points between left and right bounds. | Error    | lanelet   | The left and right bounds of a lanelet should not share any points. Shared points indicate a degenerate or malformed lanelet geometry. | Verify that the left and right bounds are defined with distinct points and do not intersect at any vertices. |

## Parameters

None.

## Related source codes

- lanelet_geometry.cpp
- lanelet_geometry.hpp
