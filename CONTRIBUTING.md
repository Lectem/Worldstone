Note: highly incomplete

# Code style

This coding style is arbitrary, as any code style, but please try to follow it.
As the project is still young, it might be subject to changes but please understand that there is no "best" coding style.

## Formating

A .clang-format file is provided, please use it before submitting. More information on clang-format [here](https://clang.llvm.org/docs/ClangFormat.html).
Once installed on your machine you can simply use the command git-clang-format that will format all the files in your staging area. Note you will need to confirm changes by marking them for add again.

## Naming convention (C/C++)

Use CamelCase for everything by default, except functions, members and variables.
Some exceptions can be made for compatibility with the STL.

* Namespaces: CamelCase `namespace WorldStone`
* Type names: CamelCase `class FileStream`
* Class members: CamelCase not starting with capital letter `char* fileName` or `void writeInt(int value)`
* Enums: CamelCase for both the enum name and members `enum class Fruit {Apple,BloodOrange};`
* Template parameters: Same rules as types `template<typename SignedResult, unsigned NbBits, typename InputType>`
* Macros: Do not use unless you can't do otherwise ! Full caps and underscored `#define WS_UNUSED`

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