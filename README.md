# Worldstone  [![Build Status](https://travis-ci.org/Lectem/Worldstone.svg?branch=master)](https://travis-ci.org/Lectem/Worldstone)[![Build status](https://ci.appveyor.com/api/projects/status/537k5bthitwtplta/branch/master?svg=true)](https://ci.appveyor.com/project/Lectem/Worldstone/branch/master)

Diablo 2 tools and engine reimplementation in modern c++.

It is written in modern C++ and aims to be portable/cross-platform.

## Status of the project 

Just started, so there's only the basic C++ tree, and a simple DC6 decoder.

## Building

### Prerequises
* CMake 3.6 or higher
* A C++14 compiler
  - GCC 6.0 - tested (5.0 should work)
  - clang 3.8 with libc++ or libstdc++-5(only 6 is tested) - tested
  - MSVC 14 (Visual 2015) - tested
 
### Copy-paste from command line (in the project directory):

    git submodule update --init
    mkdir build && cd build
    cmake ..
    cmake --build .

### Configuration

You can tweak the configuration using the following CMake variable (I suggest using cmake-gui)
* ENABLE_LTO : Use link time optimization on non debug builds if available

## Short-term objectives
* DC6/DCC viewer
* DT1 viewer

For a more detailed list of what will need to do to have an engine, check the [TODO](TODO.md) list.
