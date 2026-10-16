# Question 4 — Emergency Medical Supply Routing

This regional health org operates several medical supply centers that facilitate distribution of health equipments to other health facilities and we need to find `minimum-distance delivery route` on road network using `Dijkstra's algorithm`.

## Build and run

```bash
gcc main.c question4.c -o routes
./routes
```

our Menu:
```
1. Display locations  
2. Find shortest route  
3. Display route distance  
4. Exit  
```
**ex:**
- Start: `Central Warehouse`  
- Destination: `Health Center C`  

and expected path uses the lowest total distance, not necessarily fewest roads.
## Design

- Locations are vertices; roads are undirected weighted edges
- Dijkstra tracks predecessor nodes so the full path can be reconstructed
- Output shows the location sequence and total distance
- Missing locations, same start/end, and unreachable destinations are handled safely

