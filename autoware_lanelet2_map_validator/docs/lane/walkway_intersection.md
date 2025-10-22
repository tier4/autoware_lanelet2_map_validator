# walkway_intersection

## Validator name

mapping.lane.walkway_intersection

## Feature

This validator ensures that walkway lanelets properly intersect with road/private lanelets and maintain adequate safety margins beyond intersection areas. It validates the spatial relationship between pedestrian pathways and vehicular roads to ensure proper pedestrian navigation zones.

The validator specifically checks:

- That walkway lanelets have conflicting relationships with at least one road/private lanelet (indicating geometric intersection)
- That walkways extend at least 3 meters beyond the intersection area with road/private lanelets

| Issue Code                   | Message                                                                                                                                                   | Severity | Primitive | Description                                                                                   | Approach                                                                                                                                   |
| ---------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- | -------- | --------- | --------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Lane.WalkwayIntersection-001 | Walkway lanelet should only exist when intersecting with road/private lanelets. No conflicts found with any road/private lanelets.                        | Error    | lanelet   | Walkway lanelets must have conflicting relationships with at least one road/private lanelet.  | Use routing graph to detect conflicts between walkway and all road/private lanelets.                                                       |
| Lane.WalkwayIntersection-002 | Walkway lanelet must extend at least 3 meters beyond the intersection with road/private lanelet {road_lanelet_id}. Current extension: {extension} meters. | Error    | lanelet   | Walkway lanelets must extend at least 3 meters from the intersection area for safety margins. | Calculate geometric intersection polygon and measure distance from intersection to walkway boundary edges using boost::geometry functions. |

## Parameters

None.

## Related source codes

- walkway_intersection.cpp
- walkway_intersection.hpp
