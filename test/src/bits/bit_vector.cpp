//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <test/sequence/concepts.hpp>        // bit_sequence
#include <xstd/bits/basic_bit_sequence.hpp>  // basic_bit_sequence
#include <xstd/bits/bit_vector.hpp>          // bit_vector
#include <xstd/bits/block_sequence.hpp>      // block_vector
#include <xstd/bits/ownership.hpp>           // ownership
#include <xstd/bits/ranges/sequence_view.hpp> // sequence_view
#include <algorithm>                         // equal
#include <concepts>                          // same_as
#include <cstddef>                           // size_t
#include <cstdint>                           // uint8_t
#include <limits>                            // numeric_limits
#include <memory>                            // allocator
#include <ranges>                            // iota, transform
#include <vector>                            // vector

BOOST_AUTO_TEST_SUITE(BitVector)

using T = xstd::bit_vector<std::uint8_t>;

// Dependent, so a constrained-away member is a false rather than a hard error.
template<class X>
constexpr bool can_grow = requires (X& x) { x.push_back(true); x.resize(1UZ); };

// std::vector<bool> under its own name: the sequence adaptor over a heap of blocks. [design.md#the-public-names]
BOOST_AUTO_TEST_CASE(TheDynamicSequenceIsTheSequenceAdaptorOverAHeapOfBlocks)
{
        static_assert(std::same_as<T, xstd::basic_bit_sequence<xstd::block_vector<std::uint8_t>, xstd::ownership::owns, false>>);
        static_assert(std::same_as<xstd::bit_vector<std::uint8_t, std::allocator<std::uint8_t>>, T>);
        static_assert(test::sequence::bit_sequence<T>);
}

// [vector]'s constructors, every shape, against std::vector<bool> built the same way.
BOOST_AUTO_TEST_CASE(ItIsBuiltLikeAStdVector)
{
        auto const pattern = std::views::iota(0UZ, 20UZ) | std::views::transform([](auto i) { return i % 3 == 0; });
        auto const model = std::vector<bool>(pattern.begin(), pattern.end());

        BOOST_CHECK(T().empty());
        BOOST_CHECK_EQUAL(T(17).size(), 17UZ);
        BOOST_CHECK(std::ranges::equal(T(5, true), std::vector<bool>(5, true)));
        BOOST_CHECK(std::ranges::equal(T(5, false), std::vector<bool>(5, false)));
        BOOST_CHECK(std::ranges::equal(T(pattern.begin(), pattern.end()), model));
        BOOST_CHECK(std::ranges::equal(T(std::from_range, pattern), model));
        BOOST_CHECK(std::ranges::equal(T{ true, false, true }, std::vector<bool>{ true, false, true }));

        auto v = T();
        v = { false, true };
        BOOST_CHECK(std::ranges::equal(v, std::vector<bool>{ false, true }));
        v.assign(3, true);
        BOOST_CHECK(std::ranges::equal(v, std::vector<bool>(3, true)));
        v.assign(pattern.begin(), pattern.end());
        BOOST_CHECK(std::ranges::equal(v, model));
        v.assign({ true });
        BOOST_CHECK(std::ranges::equal(v, std::vector<bool>{ true }));
}

// Growth is the owner's: push, pop, emplace, resize, reserve, shrink and clear, each against the model.
BOOST_AUTO_TEST_CASE(ItGrowsLikeAStdVector)
{
        auto v = T();
        auto m = std::vector<bool>();
        for (auto i = 0UZ; i < 30UZ; ++i) {
                v.push_back(i % 2 == 0);
                m.push_back(i % 2 == 0);
        }
        BOOST_CHECK(std::ranges::equal(v, m));
        BOOST_CHECK_EQUAL(static_cast<bool>(v.emplace_back(true)), true);
        v.pop_back();
        BOOST_CHECK(std::ranges::equal(v, m));

        v.resize(40, true);
        m.resize(40, true);
        BOOST_CHECK(std::ranges::equal(v, m));
        v.resize(7);
        m.resize(7);
        BOOST_CHECK(std::ranges::equal(v, m));

        v.reserve(100);
        BOOST_CHECK_GE(v.capacity(), 100UZ);
        BOOST_CHECK(std::ranges::equal(v, m));
        v.shrink_to_fit();
        BOOST_CHECK_GE(v.capacity(), v.size());

        BOOST_CHECK_EQUAL(v.max_size(), std::numeric_limits<std::size_t>::max());

        v.clear();
        BOOST_CHECK(v.empty());
        BOOST_CHECK(v == T());
}

// The view over it refers into the block_vector and cannot grow it. [design.md#views-over-owners]
BOOST_AUTO_TEST_CASE(AViewOverItCannotGrowIt)
{
        auto v = T(5);
        auto const s = xstd::sequence_view(v);
        s[2] = true;
        BOOST_CHECK(static_cast<bool>(v[2]));

        static_assert(not can_grow<decltype(s)>);
        static_assert(    can_grow<T>);
}

BOOST_AUTO_TEST_SUITE_END()
