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

## Running

```bash
./server
```

## To Do
- [ ] compilation with `-DDEBUG`
- [ ] documentation with `doxygen`
- [ ] define "excessive length"
- [ ] receiver server
    - [ ] forward messages
    - [ ] handle multiple clients
- [ ] own test cases
    - [ ] invalid length
    - [ ] invalid padding
    - [ ] messages in correct order
