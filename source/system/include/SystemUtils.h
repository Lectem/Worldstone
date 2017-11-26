/**
 * @file SystemUtils.h
 * @author Lectem
 * @date 08/10/2017.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <bitset>
#include <limits.h>
#include <type_traits>
#include "Platform.h"

/// Mark a variable as unused to shut warnings when it is unused on purpose.
#define WS_UNUSED(x) (void)x

namespace WorldStone
{
namespace Utils
{

/** Convert a N-bits 2's complement signed value to a SignedResult value.
 * @tparam SignedResult Type of the result, must be signed.
 * @tparam NbBits The size in bits of value. Value must be between > 1.
 * @param value The original value, 2's complement on @ref NbBits bits.
 * @test{System,SignExtend}
 */
template<typename SignedResult, unsigned NbBits, typename InputType>
inline SignedResult signExtend(const InputType value)
{
    static_assert(std::is_integral<SignedResult>::value && std::is_integral<InputType>::value, "");
    static_assert(NbBits > 1, "Can not represent a signed value on 1 bit.");
    static_assert(NbBits <= sizeof(SignedResult) * CHAR_BIT,
                  "Result type is not big enough to hold the values.");
    static_assert(std::is_signed<SignedResult>::value, "Result type must be signed");
    struct
    {
        SignedResult x : NbBits;
    } s;
    return s.x = static_cast<SignedResult>(value);
}

#if 0
// Safe version of signExtend that does not rely on implementation defined unions.
template <typename SignedResult, unsigned NbBits, typename InputType>
inline SignedResult signExtendS(InputType value)
{
    static_assert(std::is_integral<SignedResult>::value && std::is_integral<InputType>::value, "");
    static_assert(NbBits > 1, "Can not represent a signed value on 1 bit.");
    static_assert(NbBits <= sizeof(SignedResult) * CHAR_BIT, "Result type is not big enough to hold the values.");
    static_assert(std::is_signed<SignedResult>::value, "Result type must be signed");
    // Note: Since c++14, `int32_t(1) << 32` is no more undefined, so InputType can be signed.
    const InputType signBitMask = (InputType(1) << (NbBits - 1) ); // Highest bit mask, also 2^bits
    const bool signBit = value & signBitMask;  // Signed or not
    const InputType valWithoutSignBit = value & (signBitMask - 1);
    // Note that this does not rely on the current platform using 2's complement
    // valWithoutSignBit - 2^bits is the 2's complement value.
    // Multiplying by signBit avoids branching. ie no 'if(signBit)'.
    return valWithoutSignBit - signBit * signBitMask;
}
#endif

/** Reverse the order of the bits in an bitset value.
 * @param bits The bits in a type that support bitset operators
 * @test{System,ReverseBits}
 */
template<typename T>
inline T reverseBits(T bits)
{
    constexpr size_t nbBits = sizeof(T) * CHAR_BIT;
#if defined(WS_CLANG)
    switch (nbBits)
    {
#if __has_builtin(__builtin_bitreverse8)
    case 8: return __builtin_bitreverse8(bits);
#endif
#if __has_builtin(__builtin_bitreverse16)
    case 16: return __builtin_bitreverse16(bits);
#endif
#if __has_builtin(__builtin_bitreverse32)
    case 32: return __builtin_bitreverse32(bits);
#endif
#if __has_builtin(__builtin_bitreverse64)
    case 64: return __builtin_bitreverse64(bits);
#endif
    default: break;
    }
#endif
    T tmp = 0;
    for (size_t bitIndex = 0; bitIndex < nbBits; bitIndex++)
    {
        tmp <<= 1;
        tmp |= bits & 1;
        bits >>= 1;
    }
    return tmp;
}

/** Counts the number of bits set to 1.
 * @test{System,PopCount}
 */
inline uint16_t popCount(uint16_t value)
{
#if defined(WS_GCC_FAMILY)
    return uint16_t(__builtin_popcount(value));
#elif defined(WS_MSC)
    return __popcnt16(value);
#else
    std::bitset<16> bset = value;
    return uint16_t(bset.count());
#endif
}

/// @overload uint32_t popCount(uint32_t value)
inline uint32_t popCount(uint32_t value)
{
#if defined(WS_GCC_FAMILY)
    return uint32_t(__builtin_popcount(value));
#elif defined(WS_MSC)
    return __popcnt(value);
#else
    std::bitset<32> bset = value;
    return uint32_t(bset.count());
#endif
}

/// @overload uint64_t popCount(uint64_t value)
inline uint64_t popCount(uint64_t value)
{
#if defined(WS_GCC_FAMILY)
    return uint64_t(__builtin_popcountll(value));
#elif defined(WS_MSC)
    return __popcnt64(value);
#else
    std::bitset<64> bset = value;
    return uint64_t(bset.count());
#endif
}

} // namespace Utils
} // namespace WorldStone
