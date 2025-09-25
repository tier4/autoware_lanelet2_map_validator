# road_shoulder

## Validator name

mapping.lane.road_shoulder

## Feature

This validator implements the VM-01-15 requirement for road shoulder lanelets. It ensures that road shoulder lanelets have proper adjacent lanelets and border configurations.

The validator specifically checks:

- That road shoulder lanelets have at least one adjacent lanelet
- That adjacent lanelets have the proper 'road' subtype when a road shoulder has only one adjacent lanelet
- That the outer boundary of a road shoulder (the side without an adjacent lanelet) is properly marked as a 'road_border'

| Issue Code            | Message                                                                                         | Severity | Primitive  | Description                                                                            | Approach                                                                   |
| --------------------- | ----------------------------------------------------------------------------------------------- | -------- | ---------- | -------------------------------------------------------------------------------------- | -------------------------------------------------------------------------- |
| Lane.RoadShoulder-001 | Road shoulder lanelet has no adjacent lanelets.                                                 | Error    | Lanelet    | Road shoulder lanelets must have at least one adjacent lanelet.                        | Check if a road shoulder has at least one adjacent lanelet on either side. |
| Lane.RoadShoulder-002 | Road shoulder with only left/right adjacent lanelet must be adjacent to a road subtype lanelet. | Error    | Lanelet    | If a road shoulder has only one adjacent lanelet, it must be a road subtype.           | Check that the single adjacent lanelet has a 'road' subtype.               |
| Lane.RoadShoulder-003 | Road shoulder with empty left/right side must have a left/right bound with road_border type.    | Error    | LineString | If a road shoulder has an empty side, the boundary on that side must be a road_border. | Check that the boundary on the empty side has a 'road_border' type.        |

## Parameters

None.

## Related source codes

- road_shoulder.cpp
- road_shoulder.hpp
