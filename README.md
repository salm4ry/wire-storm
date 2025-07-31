# Operation WIRE STORM

## Compilation

```bash
make
```

### Environment Variables

| variable | `=1` |
| -- | -- |
| `SCAN_BUILD` | compile with `scan-build` for static analysis |
| `LLVM` | use `clang` instead of `gcc` for compilation |
| `DEBUG` | compile in debug mode (`-DDEBUG`) |

## Running

```bash
./server
```

### Usage

`./server [OPTIONS]`

- `-e, --extended`: use extended CTMP
- `-n, --num-workers <NUM>`: max number of client worker threads to use
- `-b, --backlog <LEN>`: backlog length for `listen(2)`
- `-t, --ttl <DURATION>`: message time to live in seconds
- `-h, --help`: print usage and exit

## To Do
- [ ] prefix log messages with timestamp
- [ ] own test cases
    - [ ] invalid length
    - [ ] invalid padding
    - [ ] messages in correct order
    - [ ] between physical machines
- [x] keep messages in the queue for a given "TTL" then remove them
  in order to avoid incorrect messages being sent
- [ ] performance analysis (`perf` etc.)
- [ ] optimisations
    - [ ] compile `include/` files into libraries and use compiler optimisation
      flags
        - e.g. `compare_times()`: improvement from `-O2`, no difference betweeen `-O2` and `-O3`
    - [ ] `inline` smaller functions?
