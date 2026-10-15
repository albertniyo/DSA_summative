# Question 2 — Network Device Command Registry

As network operations team, we needed to manage embedded network devices from routers, gateways to industrial controllers and since devices supports set of configured commands we need  to loads approved device commands into a Binary Search Tree and verifies technician input.

## Build and run

```bash
gcc main.c question2.c -o cmd_reg
./cmd_reg
```

Then we leverage `commands.txt` in the same directory to load, check and suggest approved commands.

## Commands

| Key | Action |
|-----|--------|
| `v` | Verify a device command |
| `p` | Print all commands (in-order / alphabetical) |
| `s` | Show number of stored commands |
| `x` | Clear the registry |
| `q` | Save session to `session_cmds.txt`, free memory, quit |

## Design

- Commands are inserted into a BST by lexicographical order.
- Exact match > `COMMAND APPROVED`.
- Unknown match > `UNKNOWN COMMAND`, optional suggestion, and an `audit.log` entry.
- Maximum of 50 approved commands.
- All nodes are freed on clear or quit.
