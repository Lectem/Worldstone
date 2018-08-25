/**
 * @file Platform.h
 * @brief Platform specific tools and macros
 * @author Lectem
 */
#pragma once
// We're disabling clang format here, as IndentPPDirectives is only in clang-format v6
// clang-format off

///@name Compiler defines
///@{

#ifdef FORCE_DOXYGEN
#   define WS_GCC_FAMILY ///< Defined if we are using gcc/clang or any gnuc compatible compiler
#   define WS_CLANG      ///< Defined if the compiler is clang
#   define WS_GCC        ///< Defined if the compiler is GCC (not clang)
#   define WS_MSC        ///< Defined if compiling using MS visual compiler (Cl.exe)

#   define WS_64BITS     ///< Defined if compiling for a 64-bits architecture
#   define WS_32BITS     ///< Defined if compiling for a 32-bits architecture
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#   define WS_GCC_FAMILY
#   if defined(__clang__)
#       define WS_CLANG
#   else
#       define WS_GCC
#   endif
#elif defined(_MSC_VER)
#   define WS_MSC
#else
#   error "Unknown compiler"
#endif

#ifdef WS_MSC
#   include <intrin.h> // MSVC requires this header to use intrinsics
#endif

#if defined(__LP64__) || defined(_WIN64)
#   define WS_64BITS
#else
#   define WS_32BITS
#endif

///@}

#define WS_STRINGIZE(_x) #_x

///@name Macros to silence warnings
/// Here is a sample on how to use those macros
///@code
/// WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU("-Wunused-parameter")
/// WS_PRAGMA_DIAGNOSTIC_IGNORED_MSC(4100) // warning C4100: '' : unreferenced formal parameter
///@endcode
///@{
#ifdef FORCE_DOXYGEN
#   define WS_PRAGMA_DIAGNOSTIC_PUSH()            ///< Saves the current state of the compiler warnings
#   define WS_PRAGMA_DIAGNOSTIC_POP()             ///< Restores the previous state of the compiler warnings
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU(_x)   ///< Silence GCC/clang-specific warnings
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)   ///< Silence GCC-specific warnings
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x) ///< Silence clang-specific warnings
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_MSC(_x)   ///< Silence MSC-specific warnings
#else
#   if defined(WS_CLANG)
#       define WS_PRAGMA_DIAGNOSTIC_PUSH()              _Pragma("clang diagnostic push")
#       define WS_PRAGMA_DIAGNOSTIC_POP()               _Pragma("clang diagnostic pop")
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)   _Pragma(WS_STRINGIZE(clang diagnostic ignored _x))
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU(_x)     WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#   else
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#   endif
#   if defined(WS_GCC)
#       define WS_PRAGMA_DIAGNOSTIC_PUSH()              _Pragma("GCC diagnostic push")
#       define WS_PRAGMA_DIAGNOSTIC_POP()               _Pragma("GCC diagnostic pop")
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)     _Pragma(WS_STRINGIZE(GCC diagnostic ignored _x) )
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU(_x)     WS_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#   else
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#   endif
#   if defined(WS_MSC)
#       define WS_PRAGMA_DIAGNOSTIC_PUSH()              __pragma(warning(push) )
#       define WS_PRAGMA_DIAGNOSTIC_POP()               __pragma(warning(pop) )
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_MSC(_x)     __pragma(warning(disable:_x) )
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU(_x)
#   else
#       define WS_PRAGMA_DIAGNOSTIC_IGNORED_MSC(_x)
#   endif

#endif
///@}

/**@def WS_FALLTHROUGH
 * @brief Equivalent to the [[fallthrough]] attribute when supported
 * Used to annotate implicit fallthroughs in a switch. Here is an example:
 * @code
 * switch(foo)
 * {
 *     case 0:
 *         doSomething();
 *         WS_FALLTHROUGH;
 *     case 1:
 *         doSomethingForBothCase0and1();
 *         break;
 *     default:
 *         break;
 * }
 * @endcode
 */
#if __cplusplus >= 201703 || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703)
#   define WS_FALLTHROUGH [[fallthrough]]
#elif defined(WS_CLANG)
#   if __cplusplus >= 201103L && defined(__has_warning)
#       if __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#           define WS_FALLTHROUGH [[clang::fallthrough]]
#       endif
#   endif
#elif defined(WS_GCC) && __GNUC__ >= 7
#   define WS_FALLTHROUGH __attribute__((fallthrough))
#endif
#ifndef WS_FALLTHROUGH
#   define WS_FALLTHROUGH ((void)0)
#endif
// clang-format on

/**
 * Types support
 */
#include <stddef.h>
#include <stdint.h>
/**User litteral for size_t (ie: size_t s = 10_z)
 * @return The integer value as a size_t
 */
constexpr size_t operator"" _z(unsigned long long int n) { return size_t(n); }
