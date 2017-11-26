/**
 * @file Platform.h
 * @author Lectem
 */
#pragma once

#if defined(__clang__)
/// Defined if the compiler is clang
#define WS_CLANG
#endif

#if defined(__GNUC__) || defined(__GNUG__)

/// Defined if we are using gcc/clang or any gnuc compatible compiler
#define WS_GCC_FAMILY

#ifndef WS_CLANG
/// Defined if the compiler is GCC (not clang)
#define WS_GCC
#endif

#elif defined(_MSC_VER)

/// Defined if compiling using MS visual compiler (Cl.exe)
#define WS_MSC

#endif

#ifdef WS_MSC
#include <intrin.h>
#endif
