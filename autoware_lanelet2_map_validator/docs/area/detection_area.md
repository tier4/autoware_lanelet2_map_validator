# detection_area

## Validator name

mapping.area.detection_area

## Feature

This validator checks whether the details in the `detection_area` subtype regulatory elements are valid.

This validator checks the following validation subjects:

- detection_area type polygons
- detection_area subtype regulatory elements

What to validate:

1. Is the polygon being referred to any regulatory element
2. Is the regulatory element referring to a "single" detection_area polygon
3. The referred polygon should be a detection_area type
4. Is the regulatory element referring to exactly one stop_line type ref_line
5. Is the regulatory element being referred by at least one road subtype lanelet
6. Is the regulatory element only being referred by road subtype lanelets

The output issue marks "polygon", "linestring", "lanelet", or "regulatory element" as the **primitive**, and the corresponding ID is written together as **ID**.

| Issue Code             | Message                                                                                    | Severity | Primitive          | Description                                                                                                                                                    | Approach                                                                       |
| ---------------------- | ------------------------------------------------------------------------------------------ | -------- | ------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| Area.DetectionArea-001 | "Detection area polygon is not being referred to by any regulatory element."               | Error    | polygon            | There is a `detection_area` type polygon that is not referred by any regulatory element.                                                                       | Add a detection_area subtype regulatory element that refers to this polygon.   |
| Area.DetectionArea-002 | "Detection area regulatory element must refer to exactly one detection_area polygon."      | Error    | regulatory element | There is a `detection_area` subtype regulatory element that does not refer to exactly one polygon.                                                             | Ensure the regulatory element refers to exactly one detection_area polygon.    |
| Area.DetectionArea-003 | "Polygon referred by detection area regulatory element must have type 'detection_area'."   | Error    | regulatory element | There is a polygon referred by a `detection_area` subtype regulatory element that is not a `detection_area` type polygon.                                      | Check that the referred polygon has type 'detection_area'.                     |
| Area.DetectionArea-004 | "Detection area regulatory element must refer to exactly one stop_line type ref_line."     | Error    | regulatory element | There is a `detection_area` subtype regulatory element that either has no ref_line, multiple ref_lines, or ref_line that is not a `stop_line` type linestring. | Ensure exactly one ref_line is set that refers to a stop_line type linestring. |
| Area.DetectionArea-005 | "Detection area regulatory element must be referred by at least one road subtype lanelet." | Error    | regulatory element | There is a `detection_area` subtype regulatory element that is not being referred by any road subtype lanelet.                                                 | Add a road subtype lanelet that refers to this regulatory element.             |
| Area.DetectionArea-006 | "Detection area regulatory element must only be referred by road subtype lanelets."        | Error    | regulatory element | There is a `detection_area` subtype regulatory element that is being referred by a non-road subtype lanelet.                                                   | Ensure only road subtype lanelets refer to this regulatory element.            |

## Parameters

None.

## Related source codes

- detection_area.cpp
- detection_area.hpp
