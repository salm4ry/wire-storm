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
- `-h, --help`: print usage and exit

## To Do
- [x] compilation with `-DDEBUG`
- [x] documentation with `doxygen`
    - [x] update for new functions
    - [x] make `index.html` show relevant files/repository README (currently blank)
- [x] make sure messages are of the correct length
- [ ] prefix log messages with timestamp
- [x] finalise error handling
- [x] receiver server
    - [x] forward valid messages
    - [x] handle multiple clients
- [x] command-line argument to choose base/extended CTMP at startup
- [ ] own test cases
    - [ ] invalid length
    - [ ] invalid padding
    - [ ] messages in correct order
    - [x] between VMs
    - [ ] between physical machines
- [ ] *new design:* one thread per receiver
    - [x] service each receiver in a separate threads (keeping track of thread
      state)
    - [x] have a maximum number of threads
    - [ ] keep receivers over the thread limit waiting for an available thread
    - [ ] keep messages in the queue for a given "grace period" then remove them
      in order to avoid incorrect messages being sent
        - [ ] store a bitmask of fds with a given message has been sent to
        - [ ] delete messages after the grace period has passed
    - [ ] configurable options
        - [ ] max number of threads
        - [ ] grace period in seconds
- [ ] performance analysis (`perf` etc.)
- [x] Wire Storm Reloaded
    - [x] updated header parsing (checksum calculation + validation)
    - [x] log errors on invalid checksum
