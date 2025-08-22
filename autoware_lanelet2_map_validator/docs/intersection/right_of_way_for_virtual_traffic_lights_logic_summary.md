# Right of Way for Virtual Traffic Lights - Logic Summary

Created the validator, with logic as shown below:

**Setup routing graph:**

- Build a RoutingGraph with Germany vehicle traffic rules
- Initialize empty issues collection

**Filter lanelets with virtual traffic lights:**

- For each lanelet, check if it has virtual_traffic_light regulatory elements
- Skip lanelets without virtual traffic lights

**Validate right-of-way regulatory elements:**

- Check if lanelet has right-of-way regulatory elements
- If no right-of-way elements: generate ISSUE-001
- If multiple right-of-way elements: generate ISSUE-002

**Validate right-of-way role assignment:**

- Get the single right-of-way regulatory element
- Check if exactly one lanelet is assigned to "RightOfWay" role
- If not exactly one: generate ISSUE-003

**Validate self-assignment:**

- Check if current lanelet assigns itself as the right-of-way role
- If not self-assigned: generate ISSUE-004

**Validate conflicting lanelets as yield:**

- Find all conflicting lanelets using routing graph relationships
- Get lanelets assigned to "Yield" role from right-of-way element
- For each conflicting lanelet not in yield role: generate ISSUE-005
