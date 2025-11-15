# Makeargs

Makefile-like argument parser for C build systems.

## Usage

Check [build.c](./build.c) for an simple example.

```sh
gcc -o build build.c
./build
```

## tests

All tests are run in the same way:

```sh
# assuming you are in a test folder
gcc -o build build.c
./build # show diff from expected output
./build save # saves output to expected
```
