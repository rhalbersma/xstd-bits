//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits.hpp>            // the whole bits surface
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE
#include <array>                    // array
#include <concepts>                 // same_as
#include <cstddef>                  // size_t
#include <cstdint>                  // uint8_t
#include <functional>               // less
#include <limits>                   // numeric_limits
#include <memory>                   // allocator
#include <ranges>                   // bidirectional_range, random_access_range

namespace {

// An enumeration whose author listed its values, which is what an enum set is keyed by.
enum class perm : std::uint8_t
{
        read,
        write,
        exec,
};

} // namespace

template<>
struct xstd::enum_traits<perm>
{
        static constexpr std::array values = {perm::read, perm::write, perm::exec};
};

namespace {

// A flag type over the same ranks, its operators returning it.
class perms : public xstd::bit_flag_set<perms, perm, std::uint8_t, 3>
{
public:
        using bit_flag_set::bit_flag_set;
};

} // namespace

// Every entity the umbrella promises, reached through it alone: no leaf test sees the umbrella at all.
BOOST_AUTO_TEST_CASE(EveryContainerArrivesThroughTheUmbrella)
{
        // The two containers that are ranges on their own terms: one indexed by position, one iterating its elements.
        static_assert(std::ranges::random_access_range<xstd::bit_array<8>>);
        static_assert(std::ranges::bidirectional_range<xstd::bit_fixed_set<8>>);

        // A view reads each owner in its own reading.
        auto const keys = xstd::bit_fixed_set<8>();
        static_assert(std::ranges::bidirectional_range<decltype(xstd::bit_set_view(keys))>);

        auto const packed = xstd::bit_array<8>();
        static_assert(std::ranges::random_access_range<decltype(xstd::bit_span(packed))>);

        // The dynamic column, one name per reading, both over a std::vector of blocks.
        static_assert(std::ranges::bidirectional_range<xstd::basic_bit_set<std::size_t, std::size_t>>);
        static_assert(std::ranges::random_access_range<xstd::basic_bit_vector<std::size_t>>);

        // The two layers the umbrella shows: basic_ chooses the storage, and the restricted name fixes size_t.
        static_assert(std::same_as<xstd::bit_fixed_set<8>, xstd::basic_bit_fixed_set<std::size_t, std::size_t, 8>>);
        static_assert(std::same_as<xstd::bit_array<8>, xstd::basic_bit_array<std::size_t, 8>>);
        static_assert(std::same_as<xstd::bit_set, xstd::basic_bit_set<std::size_t, std::size_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, std::allocator<std::size_t>>>); // NOLINT(modernize-use-transparent-functors): the default comparator, spelled to reach the allocator
        static_assert(std::same_as<xstd::bit_vector, xstd::basic_bit_vector<std::size_t, std::allocator<std::size_t>>>);

        // The enum set is the fixed set keyed by an enumeration's ranks, in the smallest block holding them.
        static_assert(std::same_as<xstd::bit_enum_set<perm>, xstd::basic_bit_fixed_set<perm, std::uint8_t, 3, xstd::bit_enum_traits<perm>>>);
        static_assert(std::same_as<decltype(xstd::basic_bit_fixed_set{perm::read}), xstd::bit_enum_set<perm>>);

        // A flag type iterates its ranks, and a mask enumeration keys a set through its one-bit values.
        static_assert(std::same_as<perms::iterator::value_type, perm>);
        static_assert(std::same_as<decltype(perms() | perms()), perms>);
        static_assert(xstd::bit_flag_traits<perm>::size == 8UZ);

        // The bounded column, the third storage point: one name per reading, each a class like the rest.
        static_assert(std::ranges::bidirectional_range<xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 8>>);
        static_assert(std::ranges::random_access_range<xstd::basic_bit_bounded_vector<std::uint8_t, 8>>);
        static_assert(std::same_as<xstd::bit_bounded_set<8>, xstd::basic_bit_bounded_set<std::size_t, std::size_t, 8>>);
        static_assert(std::same_as<xstd::bit_bounded_vector<8>, xstd::basic_bit_bounded_vector<std::size_t, 8>>);
        static_assert(std::same_as<xstd::aligned::bit_bounded_set<9>, xstd::bit_bounded_set<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::bit_bounded_vector<9>, xstd::bit_bounded_vector<std::numeric_limits<std::size_t>::digits>>);

        // Every name with an N at compile time has an aligned form, the width or capacity rounded up to whole blocks.
        static_assert(std::same_as<xstd::aligned::bit_fixed_set<9>, xstd::bit_fixed_set<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::bit_array<9>, xstd::bit_array<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::basic_bit_array<std::uint8_t, 9>, xstd::basic_bit_array<std::uint8_t, 16>>);
        static_assert(std::same_as<xstd::aligned::basic_bit_array<std::uint8_t, 0>, xstd::basic_bit_array<std::uint8_t, 0>>);
}
