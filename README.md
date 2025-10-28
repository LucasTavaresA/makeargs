# Makeargs

Makefile-like argument parser for C build systems.

## Usage

Check [build.c](./build.c) for an simple example.

```sh
gcc -o build build.c
./build
```

## tests

```sh
./tests # runs tests, runs `diff output expected` after
./tests save # runs tests, saving output to expected file
```
