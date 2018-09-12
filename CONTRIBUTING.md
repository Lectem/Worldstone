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
  * constexpr
  * ...

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

# Doxygen

## Comments and commands style

Use C-style comments if the comments is on multiple lines:

```cpp
/**This is a brief comment.
 * @param a This is a param description
 *
 * A longer description of this item.
 */
void foo(int a);
```

You can use C++-style comments for one-liners:
```cpp
/// A function that does nothing
void bar(){}

struct IntegerStruct
{
    int value; ///< The value of the integer
};
```

Use the 'at' character for commands:

```cpp
/// @brief Some brief description.
```

## Tests

Some Doxygen ALIASES are used for tests.

The tested function or class must use the `@test{Module,TestShortName}` command.
**Module** and **TestShortName** must not contain whitespaces

The tests are documented using the `@testimpl{TestedObject,TestShortName}` or `@testimpl{TestShortName}`(in case of multiple elements being tested at once).
**TestedObject** will be used to create a "see also" block linking to the tested object/function.

### Example:

```cpp
/////////////// Header ////////////////////

/**A class representing a file stream
 * @test{System,RO_filestream}
 * @test{System,WO_filestream}
 */
class FileStream;

//////////////// Test /////////////////////

///@testimpl{FileStream,RO_filestream}
TEST_CASE("Read-only filestream");

///@testimpl{FileStream,WO_filestream}
TEST_CASE("Write-only filestream");
```