/**
 * @file SystemUtils.h
 * @author Lectem
 * @date 08/10/2017.
 */
#pragma once

#include <stdint.h>
#include <limits.h>
#include <type_traits>

namespace WorldStone
{
namespace Utils
{

/** Convert a N-bits 2's complement signed value to a @ref SignedResult value
 * @tparam SignedResult Type of the result, must be signed.
 * @tparam NbBits The size in bits of value. Value must be between > 1.
 * @param value The original value, 2's complement on @ref NbBits bits.
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
/// Safe version of signExtend that does not rely on implementation defined unions.
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
} // namespace Utils
} // namespace WorldStone
