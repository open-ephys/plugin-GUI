/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

//==============================================================================
/**
    Macro to enable bitwise operations for scoped enums (enum struct/class).

    To use this, add the line JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS (MyEnum)
    after your enum declaration at file scope level.

    e.g. @code

    enum class MyEnum
    {
        one     = 1 << 0,
        two     = 1 << 1,
        three   = 1 << 2
    };

    JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS (MyEnum)

    MyEnum e = MyEnum::one | MyEnum::two;

    bool hasTwo = (e & MyEnum::two) != MyEnum{}; // true
    bool hasTwo = hasBitValueSet (e, MyEnum::two); // true

    e = withBitValueCleared (e, MyEnum::two);

    bool hasTwo = hasBitValueSet (e, MyEnum::two); // false

    @endcode
*/
#define JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS(EnumType)               \
    static_assert (std::is_enum_v<EnumType>,                               \
                   "JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS "           \
                   "should only be used with enum types");                 \
    constexpr auto operator& (EnumType a, EnumType b)                      \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (base_type (a) & base_type (b));      \
    }                                                                      \
    constexpr auto operator| (EnumType a, EnumType b)                      \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (base_type (a) | base_type (b));      \
    }                                                                      \
    constexpr auto operator~ (EnumType a)                                  \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (~base_type (a));                     \
    }                                                                      \
    constexpr auto& operator|= (EnumType& a, EnumType b)                   \
    {                                                                      \
        a = (a | b);                                                       \
        return a;                                                          \
    }                                                                      \
    constexpr auto& operator&= (EnumType& a, EnumType b)                   \
    {                                                                      \
        a = (a & b);                                                       \
        return a;                                                          \
    }


namespace juce
{

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, int> = 0>
constexpr bool hasBitValueSet (EnumType enumValue, EnumType valueToLookFor) noexcept
{
    return (enumValue & valueToLookFor) != EnumType{};
}

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, int> = 0>
constexpr EnumType withBitValueSet (EnumType enumValue, EnumType valueToAdd) noexcept
{
    return enumValue | valueToAdd;
}

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, int> = 0>
constexpr EnumType withBitValueCleared (EnumType enumValue, EnumType valueToRemove) noexcept
{
    return enumValue & ~valueToRemove;
}
}
