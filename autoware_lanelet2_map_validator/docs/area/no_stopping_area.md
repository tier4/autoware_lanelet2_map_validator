# no_stopping_area

## Validator name

mapping.area.no_stopping_area

## Feature

This validator checks vm-06-04 no stopping sections requirements.

Validates the relationship between no_stopping_area type polygons, their corresponding no_stopping_area subtype regulatory elements, and associated stop_line linestrings.

| Issue Code              | Message                                                                                           | Severity | Primitive          | Description                                            | Approach                       |
| ----------------------- | ------------------------------------------------------------------------------------------------- | -------- | ------------------ | ------------------------------------------------------ | ------------------------------ |
| Area.NoStoppingArea-001 | This no_stopping_area polygon is not referred by any regulatory element.                          | Error    | polygon            | Polygon is not referenced by regulatory element        | Check if polygon is referenced |
| Area.NoStoppingArea-002 | This no_stopping_area regulatory element must refer to exactly one polygon.                       | Error    | regulatory element | Regulatory element refers to wrong number of polygons  | Count referenced polygons      |
| Area.NoStoppingArea-003 | This no_stopping_area regulatory element refers to a polygon that is not a no_stopping_area type. | Error    | regulatory element | Referenced polygon has wrong type                      | Check polygon type             |
| Area.NoStoppingArea-004 | This no_stopping_area regulatory element must refer to exactly one ref_line.                      | Error    | regulatory element | Regulatory element refers to wrong number of ref_lines | Count referenced ref_lines     |
| Area.NoStoppingArea-005 | This no_stopping_area regulatory element refers to a linestring that is not a stop_line type.     | Error    | regulatory element | Referenced linestring has wrong type                   | Check linestring type          |
| Area.NoStoppingArea-006 | This no_stopping_area regulatory element is not referred by any road subtype lanelet.             | Error    | regulatory element | Regulatory element not used by road lanelet            | Check lanelet references       |

## Parameters

None.

## Related source codes

- no_stopping_area.cpp
- no_stopping_area.hpp
