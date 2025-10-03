# no_parking_area

## Validator name

mapping.area.no_parking_area

## Feature

This validator checks the validity of no_parking_area type polygons and their associated regulatory elements. It ensures proper relationships between no_parking_area polygons, regulatory elements, and road lanelets.

| Issue Code                       | Message                                                           | Severity | Primitive         | Description                                                                                          | Approach                                                                             |
| -------------------------------- | ----------------------------------------------------------------- | -------- | ----------------- | ---------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ |
| mapping.area.no_parking_area-001 | no_parking_area polygon is not referred by any regulatory element | Error    | Polygon           | A no_parking_area type polygon exists but is not referenced by any regulatory element                | Create a regulatory element with no_parking_area subtype that refers to this polygon |
| mapping.area.no_parking_area-002 | Regulatory element should refer to exactly one polygon            | Error    | RegulatoryElement | A no_parking_area subtype regulatory element refers to multiple polygons instead of exactly one      | Split into separate regulatory elements, each referring to a single polygon          |
| mapping.area.no_parking_area-003 | The referred polygon should be a no_parking_area type             | Error    | RegulatoryElement | A no_parking_area subtype regulatory element refers to a polygon that is not of type no_parking_area | Update the regulatory element to refer to the correct polygon type                   |
| mapping.area.no_parking_area-004 | Regulatory element is not referred by any road lanelet            | Error    | RegulatoryElement | A no_parking_area subtype regulatory element is not referenced by any road subtype lanelet           | Associate the regulatory element with the appropriate road lanelet                   |

## Parameters

None.

## Related source codes

- no_parking_area.cpp
- no_parking_area.hpp
