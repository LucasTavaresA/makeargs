# Makeargs

Makefile-like single-header library for building projects with C.

## Features

- **Dependency solving**: Solves and rebuilds old targets based on input/output chains, supports glob patterns
- **Variable management**: Set, get, and override variables from environment, command-line, or C code
- **Command-line flags**: Support for dry-run, always-run, other make flags, and customizable flags
- **Cross-platform**: Works on linux, didn't test but should work on Windows ⚠️

## Usage

Check [build.c](./build.c) for an simple example.

```sh
gcc -o build build.c
./build
```

## Tests

All tests are run in the same way:

```sh
# assuming you are in a test folder
gcc -o build build.c
./build # show diff from expected output
./build save # saves output to expected
```

## Contributing

To check formatting and run all the tests:

```sh
.githooks/pre-commit
```
