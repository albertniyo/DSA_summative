# Question 3 — Server Dependency Network Analyzer

Technology company operates collection of servers that needs to communicate with one another to provide services to users, so, our task was to model servers to analyze their relationships or directed dependencies with an adjacency matrix.

## Build and run

```bash
gcc main.c question3.c -o analyzer
./analyzer
```

And by default network from the brief is loaded at startup

## Commands

| Key | Action |
|-----|--------|
| `a` | Add server |
| `r` | Remove server |
| `c` | Create dependency |
| `d` | Delete dependency |
| `q` | Query server dependencies |
| `m` | Display adjacency matrix |
| `x` | Exit |

## Design

- Vertices represent servers, directed edges represent dependencies (like `A -> B` means A depends on B)
- Query shows outgoing (depends on) and incoming (depended on by) neighbors
- Removing a server also clears all connected edges
- Duplicate servers/edges and missing names are rejected safely
