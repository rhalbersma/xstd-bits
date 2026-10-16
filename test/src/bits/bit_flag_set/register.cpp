//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flag/p4313.hpp>           // bitmask_type, operator&, operator^, operator|, operator~
#include <xstd/bits/bit/bit_convert.hpp> // bit_convert
#include <xstd/bits/bit_flag_set.hpp>    // bit_flag_set
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                     // ranges::equal
#include <array>                         // array
#include <cstdint>                       // uint32_t
#include <ranges>                        // range

namespace dev {

// The interrupt flags of a 32-bit device register, the highest at its top bit.
enum class irq : std::uint32_t
{
        rx  = 1U << 0U,
        tx  = 1U << 1U,
        err = 1U << 31U,
};

} // namespace dev

template<>
inline constexpr bool test::p4313::bitmask_type<dev::irq> = true;

BOOST_AUTO_TEST_SUITE(BitFlagSet)
BOOST_AUTO_TEST_SUITE(Register)

namespace {

using dev::irq;
using irqs = xstd::bit_flag_set<irq>;

} // namespace

// A flag type spans the register, and a volatile one is no range, so no algorithm walks the register bit by bit.
BOOST_AUTO_TEST_CASE(TheFlagTypeSpansTheRegisterAndIsNoRangeWhenVolatile)
{
        static_assert(irqs::max_size() == 32UZ and sizeof(irqs) == sizeof(std::uint32_t));
        static_assert(std::ranges::range<irqs const&> and not std::ranges::range<irqs volatile&>);
        BOOST_CHECK(true);
}

// A status register is read once into a flag set, which is changed as a value and written to a control register once.
BOOST_AUTO_TEST_CASE(AStatusRegisterRoundTripsThroughAFlagSet)
{
        std::uint32_t volatile status  = (1U << 31U) | 1U;
        std::uint32_t volatile control = 0U;

        auto flags = xstd::bit_convert<irqs>(status);
        BOOST_CHECK(std::ranges::equal(flags, std::array{irq::err, irq::rx}));
        BOOST_CHECK(flags.contains(irq::err) and not flags.contains(irq::tx));

        flags.insert(irq::tx);
        control = xstd::bit_convert<std::uint32_t>(flags);
        BOOST_CHECK_EQUAL(control, (1U << 31U) | 0b11U);
        BOOST_CHECK(xstd::bit_convert<irqs>(control) == (irqs{irq::err, irq::tx, irq::rx}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
