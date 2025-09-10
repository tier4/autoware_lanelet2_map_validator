# buffer_zone_validity

## Validator name

mapping.area.buffer_zone_validity

## Feature

This validator checks that buffer zones (polygons with type "hatched_road_markings") meet the following validity criteria:

1. All points in a buffer zone polygon must be shared with nearby lanelets
2. Buffer zones should not partially overlap with intersection areas
3. Buffer zone polygons must be geometrically valid according to boost::geometry::is_valid

[Vector Map Requirements vm-06-01](https://docs.pilot.auto/en/reference-design/common/map-requirements/vector-map-requirements/category_area/#vm-06-01-buffer-zone).

| Issue Code                  | Message                                                                                                       | Severity | Primitive | Description                                                                                   | Approach                                                                                                                |
| --------------------------- | ------------------------------------------------------------------------------------------------------------- | -------- | --------- | --------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| Area.BufferZoneValidity-001 | Buffer zone polygon has points not shared with any nearby lanelet. (Point IDs: {point_ids})                   | Error    | Polygon   | Identifies buffer zones that have points which aren't shared with adjacent lanelet boundaries | Ensure all points in the buffer zone polygon are shared with nearby lanelet boundaries                                  |
| Area.BufferZoneValidity-002 | Buffer zone polygon partially overlaps with intersection area (Intersection Area ID: {intersection_area_id}). | Error    | Polygon   | Identifies buffer zones that partially overlap with intersection areas                        | Buffer zones should either be completely within intersection areas, completely outside of them, or not intersect at all |
| Area.BufferZoneValidity-003 | Buffer zone polygon is not a valid geometry: {boost_geometry_message}                                         | Error    | Polygon   | Identifies buffer zones that have invalid geometries such as self-intersections               | Fix the polygon geometry based on the specific error message provided by Boost Geometry                                 |

## Parameters

None.

## Related source codes

- buffer_zone_validity.cpp
- buffer_zone_validity.hpp
