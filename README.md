# Operation WIRE STORM

## Compilation

```bash
make
```

Environment variables:

| variable | `=1` |
| -- | -- |
| `SCAN_BUILD` | compile with `scan-build` for static analysis |
| `LLVM` | use `clang` instead of `gcc` for compilation |
| `DEBUG` | compile in debug mode (`-DDEBUG`) |

## Running

```bash
./server
```

## To Do
- [x] compilation with `-DDEBUG`
- [x] documentation with `doxygen`
    - [x] update for new functions
- [ ] define "excessive length"
- [x] finalise error handling
- [x] receiver server
    - [x] forward valid messages
    - [x] handle multiple clients
- [ ] own test cases
    - [ ] invalid length
    - [ ] invalid padding
    - [ ] messages in correct order
- [ ] Wire Storm Reloaded
    - [ ] updated header parsing (checksum calculation + validation)
    - [ ] log errors on invalid checksum
