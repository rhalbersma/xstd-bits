//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/concepts.hpp>                 // bit_set
#include <xstd/bits/ext/boost/bit_small_set.hpp> // basic_bit_small_set, bit_small_set
#include <boost/test/unit_test.hpp>              // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                             // ranges::equal
#include <concepts>                              // regular, same_as
#include <cstddef>                               // size_t
#include <cstdint>                               // uint8_t
#include <memory>                                // allocator
#include <ranges>                                // bidirectional_range
#include <set>                                   // set

BOOST_AUTO_TEST_SUITE(ExtBoostBitSmallSet)

namespace {

inline constexpr auto N = 256UZ;
using SmallSet = xstd::bit_small_set<N>;

// An allocation count the container cannot reach around, which is what tells an inline width from a heap one.
inline auto allocations = 0;

template<class T>
struct counting_allocator
{
        using value_type = T;

        counting_allocator() noexcept = default;

        template<class U>
        explicit constexpr counting_allocator(counting_allocator<U> const&) noexcept
        {}

        [[nodiscard]] auto allocate(std::size_t n)
                -> T*
        {
                ++allocations;
                return std::allocator<T>().allocate(n);
        }

        auto deallocate(T* p, std::size_t n) noexcept
                -> void
        {
                std::allocator<T>().deallocate(p, n);
        }

        [[nodiscard]] friend auto operator==(counting_allocator const&, counting_allocator const&) noexcept -> bool = default;
};

using CountedSet = xstd::basic_bit_small_set<std::size_t, N, counting_allocator<std::size_t>>;

} // namespace

// The short name is the general one at its defaults, and the allocator defaulted to is the storage's own.
BOOST_AUTO_TEST_CASE(TheShortNameIsTheGeneralOneAtItsDefaults)
{
        static_assert(std::same_as<SmallSet, xstd::basic_bit_small_set<std::size_t, N, boost::container::new_allocator<std::size_t>>>);
        static_assert(test::set::bit_set<SmallSet>);
        static_assert(std::ranges::bidirectional_range<SmallSet>);
        static_assert(std::regular<xstd::basic_bit_small_set<std::uint8_t, 24>>);
        BOOST_CHECK(true);
}

// Keys on both sides of the inline capacity, because that boundary is what this storage has and the others do not.
BOOST_AUTO_TEST_CASE(TheSetReadingAnswersStdSetAcrossTheBoundary)
{
        auto oracle = std::set<std::size_t>();
        auto ours = SmallSet();
        for (auto const i : {0UZ, 1UZ, 63UZ, 64UZ, 255UZ, 256UZ, 1000UZ}) {
                oracle.insert(i);
                ours.insert(i);
        }
        BOOST_CHECK_EQUAL(ours.size(), oracle.size());
        BOOST_CHECK(std::ranges::equal(ours, oracle));

        ours.erase(64);
        oracle.erase(64);
        BOOST_CHECK(std::ranges::equal(ours, oracle));
}

// The inline buffer is the only reason to reach for this storage, so the test is that a narrow width never allocates.
BOOST_AUTO_TEST_CASE(AWidthInsideTheCapacityStaysOffTheHeap)
{
        allocations = 0;
        {
                auto inside = CountedSet();
                inside.insert(3);
                inside.insert(N - 1);
                BOOST_CHECK(inside.contains(3) and inside.contains(N - 1));
        }
        BOOST_CHECK_EQUAL(allocations, 0);

        allocations = 0;
        {
                auto beyond = CountedSet();
                beyond.insert(5 * N);
                BOOST_CHECK(beyond.contains(5 * N));
        }
        BOOST_CHECK(allocations > 0);
}

BOOST_AUTO_TEST_SUITE_END()
