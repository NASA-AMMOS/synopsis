# cFS Integration

This folder contains additional files that enable a slightly modified build of the SYNOPSIS core library. With these modifications, it's possible to build a cFS-compatible version of the core SYNOPSIS code and run it as a cFS application.

## Instructions
1. Replace the `CMakeLists.txt` at the root repository level with this one.
1. Add the `itc_synopsis_bridge.h` file to the `include` directory.
1. Add the `itc_synopsis_bridge.cpp` file to the `src` directory.
1. Build the app according to the README instructions at the root of this repository.