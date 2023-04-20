# libtcc-example

Examples of evaluating C code during runtime, using libtcc.

## Installation

```
git clone --recursive https://github.com/saikyun/libtcc-example
cd libtcc-example
cd lib/tinycc
./configure
make
make test    ## fails on macos monterey, but examples still work
cd ..
```

## Usage

```
make run
```

On windows, you'll have to set up the compiler flags yourself. If you figure it out, please send a PR. :)