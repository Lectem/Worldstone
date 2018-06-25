/**
 * @file Platform.h
 * @author Lectem
 */
#pragma once
// We're disabling clang format here, as IndentPPDirectives is only in clang-format v6
// clang-format off
#if defined(__clang__)
#   define WS_CLANG      ///< Defined if the compiler is clang
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#   define WS_GCC_FAMILY ///< Defined if we are using gcc/clang or any gnuc compatible compiler
#   ifndef WS_CLANG
#       define WS_GCC    ///< Defined if the compiler is GCC (not clang)
#   endif
#elif defined(_MSC_VER)
#   define WS_MSC        ///< Defined if compiling using MS visual compiler (Cl.exe)
#endif

#ifdef WS_MSC
#   include <intrin.h>
#endif

#if defined(__x86_64__) || defined(_WIN64)
#   define WS_64BITS
#else
#   define WS_32BITS
#endif

#define WS_STRINGIZE(_x) #_x

#if defined(WS_CLANG)
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)   _Pragma(WS_STRINGIZE(clang diagnostic ignored _x))
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU(_x)     WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#else
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#endif
#if defined(WS_GCC)
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)     _Pragma(WS_STRINGIZE(GCC diagnostic ignored _x) )
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU(_x)     WS_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#else
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#endif
#if defined(WS_MSC)
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_MSC(_x)     __pragma(warning(disable:_x) )
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_GNU(_x)
#else
#   define WS_PRAGMA_DIAGNOSTIC_IGNORED_MSC(_x)
#endif

/**@def WS_FALLTHROUGH
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
#ifdef WS_CLANG
#   if __cplusplus >= 201103L && defined(__has_warning)
#       if __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#           define WS_FALLTHROUGH [[clang::fallthrough]]
#       endif
#   endif
#elif defined(WS_GCC) && __GNUC__ >= 7
#   define WS_FALLTHROUGH __attribute__((fallthrough))
#else
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
