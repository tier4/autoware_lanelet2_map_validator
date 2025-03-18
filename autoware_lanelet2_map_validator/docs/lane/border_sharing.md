# border_sharing

## Validator name

mapping.lane.border_sharing

## Feature

This validator checks whether physically adjacent lanelets share the same linestring as their border.
See Autoware Documentation [vm-01-03 Linestring sharing](https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-architecture/map/map-requirements/vector-map-requirements-overview/category_lane/#vm-01-03-linestring-sharing) and [vm-01-04 Sharing of the centerline of lanes for opposing traffic](https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-architecture/map/map-requirements/vector-map-requirements-overview/category_lane/#vm-01-04-sharing-of-the-centerline-of-lanes-for-opposing-traffic) for more information.

This validator may misjudge lanelets that are very close but not adjacent in the real world.

The validator will output the following issues with the corresponding primitive ID. Read [Algorithm](#algorithm) to understand the difference between "Lane.BorderSharing-001" and "Lane.BorderSharing-002".

| Issue Code             | Message                                                                                   | Severity | Primitive | Description                                                                                                                                                                                                                                                                                                                            | Approach                                                                                                             |
| ---------------------- | ----------------------------------------------------------------------------------------- | -------- | --------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| Lane.BorderSharing-001 | "Seems to be adjacent with Lanelet \<ID\> but doesn't share a border linestring."         | Error    | Lanelet   | The validator judged that the lanelet and lanelet \<ID\> is adjacent but doesn't share a border linestring. **Note that the same issue will be created against lanelet \<ID\> so two issues rise from one violation.**                                                                                                                 | Create a common border linestring of both lanelets and set the left bound or right bound of them to that linestring. |
| Lane.BorderSharing-002 | "Seems to be adjacent with Lanelet \<multiple IDs\> but doesn't share a border linestring | Error    | Lanelet   | The validator judged that the lanelet and lanelet \<ID\> is adjacent but doesn't share border linestrings. The difference with the above is that this violation may be caused from incorrect lanelet division, and the issue will be made only for the largest lanelet. (The same issue will not be made for lanelet \<multiple IDs\>) | Divide this lanelet properly and then set a common border linestring to each adjacent lanelet.                       |

## Algorithm

### Overview

The overview of the algorithm is illustrated in the flow chart below.

![flow_chart](../../media/border_sharing_flow_chart.drawio.svg)

After we get a set of pairs violating the border sharing rule, we will classify whether a pair has its opposite pair. If the validator finds both pairs (Lanelet1, Lanelet2) and (Lanelet2, Lanelet1), it means that the expanded polygon covers the border ofthe other and vise versa which will be classified to issue code `Lane.BorderSharing-001`. If there is only one pair and the opposite doesn't exist, then this issue will be classifed as `Lane.BorderSharing-002`. If there are pairs like (Lanelet 1, Lanelet2) and (Lanelet 1, Lanelet 3) but (Lanelet 2, Lanelet 1) and (Lanelet 3, Lanelet 1) don't exist, everything will be summarize to one issue of Lanelet 1, whose issue message mentioning about Lanelet 2 and 3.

### Example Case 1

Let's take a look at the example below, where Lanelet D, E, and F should share the border linestring with Lanelet G, H, and I respectively.

![simple_example](../../media/border_sharing_simple_example.drawio.svg)

When we focus on validating Lanelet E, the `mapping.lane.border_sharing` will first create an expanded polygon from Lanelet E, and collect intersecting lanelets A to I as candidates. Next, we obtain the relations with Lanelet E shown in this table.

| Lanelet               | Relation  |
| --------------------- | --------- |
| Lanelet B             | Left      |
| Lanelet D             | Previous  |
| Lanelet F             | Successor |
| Lanelet A, C, G, H, F | None      |

According to the flow chart, Lanelet B, D, and F is omitted from the candidates, and the left/right borders of the remaining lanelets are examined whether they are completely covered by the polygon. Then we can see only the border of Lanelet H covered by the expanded polygon. In addition, we will obtain the same result on validating Lanelet H that the expanded polygon covers the border of Lanelet E, which causes the issue of issue code `Lane.BorderSharing-001` for both Lanelets E and H.

### Example Case 2

Let's see another example. where the lanelet division is different with the adjacent ones like below.

![wrong_division](../../media/border_sharing_wrong_division.drawio.svg)

In this case, the expanded polygon of Lanelet X covers the border of Lanelet Y and Z, but the expaneded polygon of Lanelet Y doesn't cover the border of Lanelet X. In this case, the issue of issue code `Lane.BorderSharing-002` will be called against Lanelet X.

## Difficult cases

Since the detection "adjacent lanelets" have some ambiguity, there are might some cases that the validator misjudges the violation of border sharing rule.

### Lanelets that are not adjacent but too close

If there are two lanelets that are designed to be separated but being very close, the validator might misjudge that they are adjacent and not having a common border linestring.

### Pseudo-bidirectional lanelets

In Autoware, lanelets are intended to be unidirectional. Therefore, when we want to design bidirectional lanelets, we might place an opposite lanelet over a lanelet like below.

![pseudo_bidirectional](../../media/border_sharing_bidirectional.drawio.svg)

This is why we have the IoU (intersection over union) calculation for conflicting lanelets since the expanded the polygon will cover the border of other lanelets. We assume that if the IoU is higher than 0.05 and the polygon covers the border linestring, it is likely to be pseudo-bidirectional lanelets like this case. In other words, this validator only judges a pair of lanelets to be adjacent when they have a small IoU.

## Related source codes

- border_sharing.cpp
- border_sharing.hpp
