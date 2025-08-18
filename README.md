# Operation WIRE STORM

## Compilation

Basic compilation:
```bash
$ make
```

See `make help` for Makefile target details.

## Running

```bash
$ make run
```

Specify command line arguments with `$ARGS` when using `make run`- e.g. to run
in extended mode:

```bash
$ ARGS='-e' make run
```

Alternatively, ensure that `./lib` is in `$LD_LIBRARY_PATH`:

```bash
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
$ ./ws_server [OPTIONS]
```


### Usage

See `./ws_server --help` or the manual page (`make man`) for available
configuration options.

> [!NOTE]
> Make sure that `/proc/sys/net/core/somaxconn` is set to at least 64 (the
> server program's maximum backlog value)- see `listen(2)` for details.

### Testing

Original test suite:

```bash
$ ./ws-server &
$ python3 wire-storm/tests.py
```

Extended CTMP test suite:

```bash
$ ./ws-server -e &  # run in extended mode
$ python3 wire-storm-reloaded-1.0.0/tests.py
```

## Documentation

Generate HTML and LaTeX documentation available using `make docs` (uses
[doxygen](https://www.doxygen.nl/index.html))

To view the HTML documentation in a browser:

```bash
$ firefox doc/html/index.html
```

## Licensing

This project is licensed under GNU GPL 2.0.
