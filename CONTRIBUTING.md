Note: highly incomplete

# Code style

A .clang-format file is provided, please use it before submitting. More information on clang-format [here](https://clang.llvm.org/docs/ClangFormat.html).
Once installed on your machine you can simply use the command git-clang-format that will format all the files in your staging area. Note you will need to confirm changes by marking them for add again.

# CMake

Please try to keep the CMakeLists as clean as possible, and follow best (known) practices.
When adding/using some external library, always add/use the alias with the `external::` if the library does not already have one.
For example :

```cmake
target_link_libraries(hello_world
    PRIVATE 
        external::fmt # Not provided, add the external:: prefix
        Qt5::Core     # No need to add external, already in its own namespace
)
```