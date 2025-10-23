# road_markings

## Validator name

mapping.intersection.road_markings

## Feature

This validator checks the 'road_marking' subtype regulatory element to ensure it correctly refers to stop_line linestrings.

According to vm-03-14, a road_marking regulatory element must refer to a single 'stop_line' type linestring using the 'refers' role (not ref_line).

| Issue Code | Message | Severity | Primitive | Description | Approach |
| ---------- | ------- | -------- | --------- | ----------- | -------- |
| Intersection.RoadMarkings-001 | This road_marking regulatory element must refer to exactly one stop_line linestring using the 'refers' role and must not have any 'ref_line'. | Error | regulatory element | The road_marking regulatory element does not have exactly one refers linestring or has ref_line members | Ensure the regulatory element has exactly one member with role 'refers' and no members with role 'ref_line' |
| Intersection.RoadMarkings-002 | This refers linestring must be of type 'stop_line'. | Error | linestring | The referred linestring is not of type stop_line | Ensure the referred linestring has type attribute set to 'stop_line' |

## Parameters

None.

## Related source codes

- road_markings.cpp
- road_markings.hpp
