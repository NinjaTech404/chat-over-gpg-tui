/*
    flags.h - helper for bitwise flag-like operations on scoped enums

    Copyright (c) 2024 Zaven Maradyan <zaven@voithos.io>
    Copyright (c) 2025 g10 Code GmbH
    Software engineering by Ingo Klöcker <dev@ingo-kloecker.de>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this source file (the "Materials"), to deal in the Materials
    without restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies of the
    Materials, and to permit persons to whom the Materials is furnished to do
    so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Materials.

    THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER
    DEALINGS IN THE MATERIALS.

    SPDX-License-Identifier: MIT
*/

#ifndef __GPGMEPP_FLAGS_H__
#define __GPGMEPP_FLAGS_H__

// Based on the BitFlags class template published at
// https://voithos.io/articles/type-safe-enum-class-bit-flags/
// under the MIT license.
// The names of the member functions were changed to match the API of std::bitset.

#include <bitset>
#include <initializer_list>
#include <iosfwd>
#include <type_traits>

namespace GpgME
{

// Helper class for bitwise flag-like operations on scoped enums.
//
// This class provides a way to represent combinations of enum values without
// directly overloading operators on the enum type itself. This approach
// avoids ambiguity in the type system and allows the enum type to continue
// representing a single value, while the Flags can hold a combination
// of enum values.
//
// Example usage:
//
// enum class DeletionFlag : unsigned int {
//     AllowSecret = (1 << 0),
//     Force = (1 << 1),
// };
// using DeletionFlags = Flags<DeletionFlag>;
//
// DeletionFlags flags = { DeletionFlag::AllowSecret, DeletionFlag::Force };
// flags.reset(DeletionFlag::Force);
// if (flags.test(DeletionFlag::AllowSecret)) {
//     // ...
// }
// flags |= DeletionFlag::Force;
//
// Optionally, you can define binary operators for two flags for more
// convenient combination of multiple flags:
//
// GPGMEPP_DEFINE_ENUM_FLAG_OPERATORS(DeletionFlags)
//
// DeletionFlags forceDeleteOfSecret = DeletionFlag::AllowSecret | DeletionFlag::Force;
template <typename Enum, std::size_t _Nb = sizeof(Enum) * 8>
class Flags {
    using UnderlyingT = std::underlying_type_t<Enum>;

public:
    using enum_type = Enum;

    static const std::size_t number_of_bits = _Nb;

    // Rule of zero: default copy/move constructor/assignment operators are fine
    constexpr inline Flags() = default;
    constexpr inline explicit Flags(Enum flag)
        : mFlags(toUnderlying(flag))
    {
    }
    constexpr inline Flags(std::initializer_list<Enum> flags)
    {
        for (Enum flag : flags) {
            mFlags |= toUnderlying(flag);
        }
    }

    static constexpr inline Flags fromUnderlyingType(UnderlyingT flags)
    {
        return Flags{flags};
    }
    constexpr inline UnderlyingT toUnderlyingType() const
    {
        return mFlags;
    }

    //! Tests if flag \a flag is set.
    constexpr inline bool test(Enum flag) const
    {
        return (mFlags & toUnderlying(flag)) == toUnderlying(flag);
    }
    //! Sets the flag \a flag if \a value is \c true. Otherwise, the flag is reset.
    constexpr inline Flags &set(Enum flag, bool value = true)
    {
        return value ? (*this |= flag) : (*this &= ~Flags(flag));
    }
    //! Sets the flag \a flag to \c false.
    constexpr inline Flags &reset(Enum flag)
    {
        return (*this &= ~Flags(flag));
    }
    //! Sets all flags to false.
    constexpr inline Flags &reset()
    {
        mFlags = static_cast<UnderlyingT>(0);
        return *this;
    }

    constexpr inline explicit operator bool() const {
        return mFlags != static_cast<UnderlyingT>(0);
    }

    friend constexpr inline Flags operator|(Flags lhs, Flags rhs) {
        return Flags(lhs.mFlags | rhs.mFlags);
    }
    friend constexpr inline Flags operator|(Flags lhs, Enum rhs) {
        return Flags(lhs.mFlags | toUnderlying(rhs));
    }
    friend constexpr inline Flags operator|(Enum lhs, Flags rhs) {
        return Flags(toUnderlying(lhs) | rhs.mFlags);
    }

    friend constexpr inline Flags operator&(Flags lhs, Flags rhs) {
        return Flags(lhs.mFlags & rhs.mFlags);
    }
    friend constexpr inline Flags operator&(Flags lhs, Enum rhs) {
        return Flags(lhs.mFlags & toUnderlying(rhs));
    }
    friend constexpr inline Flags operator&(Enum lhs, Flags rhs) {
        return Flags(toUnderlying(lhs) & rhs.mFlags);
    }

    friend constexpr inline Flags operator^(Flags lhs, Flags rhs) {
        return Flags(lhs.mFlags ^ rhs.mFlags);
    }
    friend constexpr inline Flags operator^(Flags lhs, Enum rhs) {
        return Flags(lhs.mFlags ^ toUnderlying(rhs));
    }
    friend constexpr inline Flags operator^(Enum lhs, Flags rhs) {
        return Flags(toUnderlying(lhs) ^ rhs.mFlags);
    }

    constexpr inline Flags &operator|=(Flags other) {
        mFlags |= other.mFlags;
        return *this;
    }
    constexpr inline Flags &operator|=(Enum flag) {
        mFlags |= toUnderlying(flag);
        return *this;
    }
    constexpr inline Flags &operator&=(Flags other) {
        mFlags &= other.mFlags;
        return *this;
    }
    constexpr inline Flags &operator&=(Enum flag) {
        mFlags &= toUnderlying(flag);
        return *this;
    }
    constexpr inline Flags &operator^=(Flags other) {
        mFlags ^= other.mFlags;
        return *this;
    }
    constexpr inline Flags &operator^=(Enum flag) {
        mFlags ^= toUnderlying(flag);
        return *this;
    }

    constexpr inline Flags operator~() const {
        return Flags(~mFlags & (~std::bitset<_Nb>()).to_ulong());
    }

    friend constexpr inline bool operator==(const Flags &lhs, const Flags &rhs) {
        return lhs.mFlags == rhs.mFlags;
    }
    friend constexpr inline bool operator!=(const Flags &lhs, const Flags &rhs) {
        return !(lhs == rhs);
    }

private:
    constexpr inline explicit Flags(UnderlyingT flags)
        : mFlags(flags)
    {
    }

    static constexpr inline UnderlyingT toUnderlying(Enum flag) {
        return static_cast<UnderlyingT>(flag);
    }

private:
    UnderlyingT mFlags = static_cast<UnderlyingT>(0);
};

#define GPGMEPP_DEFINE_ENUM_FLAG_OPERATORS(FlagsType) \
constexpr inline FlagsType operator|(FlagsType::enum_type lhs, FlagsType::enum_type rhs) \
{ \
    return FlagsType{lhs} | rhs; \
} \
constexpr inline FlagsType operator&(FlagsType::enum_type lhs, FlagsType::enum_type rhs) \
{ \
    return FlagsType{lhs} & rhs; \
} \
constexpr inline FlagsType operator^(FlagsType::enum_type lhs, FlagsType::enum_type rhs) \
{ \
    return FlagsType{lhs} ^ rhs; \
}

template <typename Enum, std::size_t _Nb>
std::ostream &operator<<(std::ostream &os, const Flags<Enum, _Nb> &flags)
{
    // Write out a bitset representation.
    os << std::bitset<_Nb>(flags.toUnderlyingType());
    return os;
}

} // namespace GpgME

#endif // __GPGMEPP_FLAGS_H__
