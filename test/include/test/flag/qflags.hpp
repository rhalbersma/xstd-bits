//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_FLAG_QFLAGS_HPP
#define TEST_FLAG_QFLAGS_HPP

#include <type_traits> // underlying_type_t

// Qt's flag idiom without Qt: QFlags<Enum>, and an alignment enumeration declared as Qt::AlignmentFlag is.
namespace test::qt {

// QFlags' members a migration translates, each as Qt 6 defines it.
template<class Enum>
class flags
{
        using int_type = std::underlying_type_t<Enum>;
        int_type m_i   = 0;

        [[nodiscard]] constexpr explicit flags(int_type i) noexcept
                : m_i(i)
        {}

public:
        [[nodiscard]] flags() = default;

        [[nodiscard]] constexpr explicit(false) flags(Enum f) noexcept
                : m_i(static_cast<int_type>(f))
        {}

        [[nodiscard]] constexpr auto toInt() const noexcept
                -> int_type
        {
                return m_i;
        }

        // Every bit of f, and for f zero, no bit at all.
        [[nodiscard]] constexpr auto testFlags(flags f) const noexcept
                -> bool
        {
                return f.m_i != 0 ? (m_i & f.m_i) == f.m_i : m_i == 0;
        }

        [[nodiscard]] constexpr auto testFlag(Enum f) const noexcept
                -> bool
        {
                return testFlags(f);
        }

        [[nodiscard]] constexpr auto testAnyFlags(flags f) const noexcept
                -> bool
        {
                return (m_i & f.m_i) != 0;
        }

        constexpr auto setFlag(Enum f, bool on = true) noexcept
                -> flags&
        {
                m_i = on ? (m_i | static_cast<int_type>(f)) : (m_i & static_cast<int_type>(~static_cast<int_type>(f)));
                return *this;
        }

        [[nodiscard]] friend auto operator==(flags, flags) -> bool = default;

        [[nodiscard]] friend constexpr auto operator|(flags a, flags b) noexcept
                -> flags
        {
                return flags(static_cast<int_type>(a.m_i | b.m_i));
        }

        [[nodiscard]] friend constexpr auto operator&(flags a, flags b) noexcept
                -> flags
        {
                return flags(static_cast<int_type>(a.m_i & b.m_i));
        }
};

// An unscoped enumeration with no fixed underlying type, its range the nine bits its enumerators need.
enum alignment_flag // NOLINT(performance-enum-size): Qt declares it with no underlying type
{
        align_left       = 0x0001,
        align_right      = 0x0002,
        align_hcenter    = 0x0004,
        align_justify    = 0x0008,
        align_absolute   = 0x0010,
        align_horizontal = align_left | align_right | align_hcenter | align_justify | align_absolute, // NOLINT(bugprone-signed-bitwise): Qt's enumerator, on int
        align_top        = 0x0020,
        align_bottom     = 0x0040,
        align_vcenter    = 0x0080,
        align_baseline   = 0x0100,
        align_vertical   = align_top | align_bottom | align_vcenter | align_baseline, // NOLINT(bugprone-signed-bitwise): Qt's enumerator, on int
        align_center     = align_vcenter | align_hcenter,                             // NOLINT(bugprone-signed-bitwise): Qt's enumerator, on int
};

// Q_DECLARE_OPERATORS_FOR_FLAGS: two enumerators combine into the flags type, never into the enumeration.
[[nodiscard]] constexpr auto operator|(alignment_flag a, alignment_flag b) noexcept
        -> flags<alignment_flag>
{
        return flags(a) | b;
}

[[nodiscard]] constexpr auto operator&(alignment_flag a, alignment_flag b) noexcept
        -> flags<alignment_flag>
{
        return flags(a) & b;
}

} // namespace test::qt

#endif // TEST_FLAG_QFLAGS_HPP
