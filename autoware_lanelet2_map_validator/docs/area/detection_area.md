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
3. Is the regulatory element referring to a "single" stop_line
4. The "stop_line" should be a stop_line type linestring
5. Is the regulatory element being referred by a road subtype lanelet

The output issue marks "polygon", "linestring", "lanelet", or "regulatory element" as the **primitive**, and the corresponding ID is written together as **ID**.

| Issue Code             | Message                                                                                    | Severity | Primitive          | Description                                                                                                                                                    | Approach                                                                                           |
| ---------------------- | ------------------------------------------------------------------------------------------ | -------- | ------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| Area.DetectionArea-001 | "Detection area regulatory element must refer to a detection_area polygon."                | Error    | regulatory element | There is a `detection_area` subtype regulatory element that has no polygon refers.                                                                             | Add a polygon refer to the regulatory element that refers to the id of the detection_area polygon. |
| Area.DetectionArea-002 | "Detection area regulatory element must refer to exactly one detection_area polygon."      | Error    | regulatory element | There is a `detection_area` subtype regulatory element that refers to multiple polygons.                                                                       | Remove extra polygon refers and keep only one detection_area polygon reference.                    |
| Area.DetectionArea-003 | "Polygon referred by detection area regulatory element must have type 'detection_area'."   | Error    | polygon            | There is a polygon referred by a `detection_area` subtype regulatory element that is not a `detection_area` type polygon.                                      | Check that the polygon has type 'detection_area'.                                                  |
| Area.DetectionArea-004 | "Detection area regulatory element must refer to exactly one stop_line type ref_line."     | Error    | regulatory element | There is a `detection_area` subtype regulatory element that either has no ref_line, multiple ref_lines, or ref_line that is not a `stop_line` type linestring. | Add exactly one ref_line to the regulatory element that refers to a stop_line type linestring.     |
| Area.DetectionArea-005 | "Detection area regulatory element must be referred by at least one road subtype lanelet." | Error    | regulatory element | There is a `detection_area` subtype regulatory element that is not being referred by any road subtype lanelet.                                                 | Add a road subtype lanelet that refers to this regulatory element.                                 |

## Parameters

None.

## Related source codes

- detection_area.cpp
- detection_area.hpp
