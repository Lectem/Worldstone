# Worldstone
[![Travis build Status](https://travis-ci.org/Lectem/Worldstone.svg?branch=master)](https://travis-ci.org/Lectem/Worldstone)
[![Appveyor build status](https://ci.appveyor.com/api/projects/status/537k5bthitwtplta/branch/master?svg=true)](https://ci.appveyor.com/project/Lectem/Worldstone/branch/master)
[![Coverage](https://codecov.io/gh/Lectem/Worldstone/branch/master/graph/badge.svg)](https://codecov.io/gh/Lectem/Worldstone)
[![CDash dashboard](https://img.shields.io/badge/CDash-Access-blue.svg)](http://my.cdash.org/index.php?project=Worldstone)
[![Pull requests](https://img.shields.io/github/issues-pr-raw/Lectem/Worldstone.svg)](https://github.com/Lectem/Worldstone/pulls)
[![Opened issues](https://img.shields.io/github/issues-raw/Lectem/Worldstone.svg)](https://github.com/Lectem/Worldstone/issues)
[![Gitter chat](https://badges.gitter.im/Lectem/Worldstone.png)](https://gitter.im/Worldstone/Lobby)
[![Documentation](https://img.shields.io/badge/Documentation-latest-blue.svg)](https://lectem.github.io/Worldstone)

Diablo 2 tools and engine reimplementation in 'modern' C++ (see [C++ Usage](#c-usage)).

It aims to be portable/cross-platform.

## Status of the project 

DC6 and DCC (sprites) decoders are now working.
Work is progressing slowly. While not reflected in the repo, a lot of documentation has been gathered for future work !

## Building

### Prerequisites
* CMake 3.6 or higher
* A C++14 compiler
  - GCC 6.0 - tested (5.0 should work)
  - clang 3.8 with libc++ or libstdc++-5(only 6 is tested) - tested
  - MSVC 14 (Visual 2015) - tested
* Bzip2 for Stormlib(libbz2-dev on debian/ubuntu)
* QT 5.x (only for the tools)
  
### Copy-paste from command line (in the project directory):

    git submodule update --init --recursive
    mkdir build && cd build
    cmake ..
    cmake --build .

### Configuration

You can tweak the configuration using the following CMake variables (I suggest using ccmake or cmake-gui)
* CMAKE_PREFIX_PATH : You might need to add your Qt path to this variable
* ENABLE_LTO : Use link time optimization on non debug builds if available
* ENABLE_WARNINGS_SETTINGS : Set to false to remove warning flags added by the project
* BUILD_TESTING : Disable all testing features
* WS_BUILD_TESTS : Disable Worldstone tests
* WS_USE_DOXYGEN : Should we (try to) add a doc target

### On Windows

- Required shared libraries should automatically be copied to the executable directory by CMake, but in case where you can't run the tools from Visual, add the QTDIR environment variable with the Qt path as value (example : C:\Qt\5.7\msvc2015_64) then add %QTDIR%\bin and %QTDIR%\plugins to your PATH variable.

## Short-term objectives
* DC6/DCC viewer
* DT1 viewer

For a more detailed list of what will need to do to have an engine, check the [TODO](TODO.md) list.

## C++ Usage

While this project tends to use modern features of C++, it tries to use them sparingly. It is not [Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b), but is still similar.
Basically, if you can do something without the latest X or Y fancy feature, then don't use the feature.

For example:

- IOstreams are banned (anybody asking why can lookup the reason easily)
- Don't use a feature just because you can. (ie. don't put lambdas, SFINAE and auto where it is not needed)
- Try not to overuse templates (too much).
- Use the good stuff !
  * static_assert
  * move-semantics
  * custom litterals
  * atomics
  * unique_ptr
  * ...
