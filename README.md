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
- [ ] fix race between adding receivers and sending a message: make sure receivers get the most recent message if it was sent *before* they were accepted
- [ ] performance analysis (`perf` etc.)
- [x] Wire Storm Reloaded
    - [x] updated header parsing (checksum calculation + validation)
    - [x] log errors on invalid checksum
