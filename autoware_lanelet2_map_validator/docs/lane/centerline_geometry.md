# centerline_geometry

## Validator name

mapping.lane.centerline_geometry

## Feature

This validator checks whether custom centerline of lanelets have valid geometry such as...

- The edge points are set on the lanelet edge
- All points of the centerline is inside the belonging lanelet
- All points are not apart from the lanelet plane

The validator will output the following issues with the corresponding primitive ID.

| Issue Code                  | Message                                                                                           | Severity | Primitive  | Description                                                                                                  | Approach                                                                                                                                               |
| --------------------------- | ------------------------------------------------------------------------------------------------- | -------- | ---------- | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Lane.CenterlineGeometry-001 | The edge point of the centerline is too far from the lanelet edge. (Point ID: {point_id})         | Error    | Linestring | The edge point of the centerline doesn't lie on the lanelet edge.                                            | Both edge points of the centerline should be set on the lanelet edge. Fix the point's location on to the lanelet edge.                                 |
| Lane.CenterlineGeometry-002 | This centerline contains points outside of the lanelet. (Point IDs: {point_ids})                  | Error    | Linestring | All points of a centerline should be inside or on the border of the belonging lanelet but some points don't. | Make sure all points of the centerline is inside the lanelet.                                                                                          |
| Lane.CenterlineGeometry-003 | This centerline contains points that are distant from the lanelet plane. (Point IDs: {point_ids}) | Error    | Linestring | All points of a centerline should be on the lanelet plane but some points don't.                             | Make sure all points of the centerline is on the lanelet plane. Be careful the points are not apart from the plane especially in the height direction. |

## Parameters

| Parameter        | Default Value | Description                                                                                                                                                                                                                                                                                                    |
| ---------------- | ------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| dimension_mode   | 3D            | Select from a string "2D" or "3D". If "3D" is selected, this validator will check all issues. If "2D" is selected, this validator will NOT check about `Lane.CenterlineGeometry-003`.                                                                                                                          |
| planar_threshold | 0.01          | The threshold to validate whether a point is outside of the lanelet polygon **in meters**. Used to validate issue `Lane.CenterlineGeometry-001`. This threshold is provided since restrict validation whether a point is inside the lanelet is difficult when the point is on the border of multiple lanelets. |
| height_threshold | 0.1           | The threshold to validate whether a point is distant from the lanelet plane **in meters**. Used to validate issue `Lane.CenterlineGeometry-003`. This threshold is provided since restrict validation whether a point is inside the lanelet is difficult when the point is on the border of multiple lanelets. |

## Related source codes

- centerline_geometry.cpp
- centerline_geometry.hpp
