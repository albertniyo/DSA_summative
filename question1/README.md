# Question 1 — Embedded Device Event Logger

The scenario is about an industrial monitoring device that collects events from sensors and hardware components during operation, and some events can be `temperature warnings`, `low-battery alerts`, `sensor failure`, `communication errors` and due to limited memory we can only stores the 25 most recent device events in a doubly linked list.

## Build and run

```bash
gcc main.c question1.c -o event
./event
```

## Commands

| Key | Action |
|-----|--------|
| `n` | Next (newer) event |
| `p` | Previous (older) event |
| `a` | Add a new event |
| `c` | Show current event |
| `x` | Clear all events |
| `q` | Save session to `session.log`, free memory, quit |

## Design
During implementations some of decision where taken to meet our vision, below are some:

- each event node has `prev` and `next` pointers
- events are appended at the tail chronologically
- When capacity exceeds 25, the oldest node (head) is freed
- Head, tail, and current pointers are updated on insert, navigate, and clear
