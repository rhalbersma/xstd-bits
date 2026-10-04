//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                 // bit_array
#include <xstd/bits/bit_bounded_set.hpp>           // basic_bit_bounded_set, bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>        // bit_bounded_vector
#include <xstd/bits/bit_set.hpp>                   // basic_bit_set, bit_set
#include <xstd/bits/bit_vector.hpp>                // basic_bit_vector, bit_vector
#include <xstd/bits/ext/boost/dynamic_bitset.hpp>  // bit_convert, bit_convertible_to
#include <boost/container/small_vector.hpp>        // small_vector
#include <boost/dynamic_bitset/dynamic_bitset.hpp> // dynamic_bitset
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <boost/version.hpp>                       // BOOST_VERSION
#include <bitset>                                  // bitset
#include <cstddef>                                 // size_t
#include <cstdint>                                 // uint8_t, uint16_t, uint64_t
#include <new>                                     // bad_alloc
#include <ranges>                                  // filter, iota, to
#include <stdexcept>                               // overflow_error
#include <utility>                                 // move
#include <vector>                                  // vector

BOOST_AUTO_TEST_SUITE(ExtBoostDynamicBitset)

namespace {

template<class T>
concept is_set = requires { typename T::key_type; };

template<class T>
[[nodiscard]] auto positions(T const& x)
        -> std::vector<std::size_t>
{
        if constexpr (is_set<T>) {
                return {x.begin(), x.end()};
        } else {
                return std::views::iota(0UZ, x.size()) | std::views::filter([&](std::size_t i) -> bool { return x[i]; }) | std::ranges::to<std::vector>();
        }
}

template<class Block>
[[nodiscard]] auto make(std::size_t width, std::vector<std::size_t> const& keys)
        -> boost::dynamic_bitset<Block>
{
        auto b = boost::dynamic_bitset<Block>(width);
        for (auto const k : keys) {
                b.set(k);
        }
        return b;
}

} // namespace

// In: the size is a sequence's width, position i stays position i, at any two block widths.
BOOST_AUTO_TEST_CASE(ADynamicBitsetConvertsInKeepingItsSizeAndPositions)
{
        auto const keys   = std::vector<std::size_t>{0, 5, 12};
        auto const narrow = make<std::uint8_t>(13, keys);

        auto const v = xstd::bit_convert<xstd::bit_vector>(narrow);
        BOOST_CHECK_EQUAL(v.size(), 13UZ);
        BOOST_CHECK(positions(v) == keys);
        BOOST_CHECK(positions(xstd::bit_convert<xstd::basic_bit_vector<std::uint8_t>>(narrow)) == keys);
        BOOST_CHECK(positions(xstd::bit_convert<xstd::basic_bit_set<std::size_t, std::uint16_t>>(narrow)) == keys);

        // A set target takes whole blocks of its own, and a wider source block lands on narrower ones.
        auto const wide = make<std::uint64_t>(70, {0, 69});
        BOOST_CHECK_EQUAL(xstd::bit_convert<xstd::bit_vector>(xstd::bit_convert<xstd::bit_set>(wide)).size(), 128UZ);
        BOOST_CHECK(positions(xstd::bit_convert<xstd::basic_bit_vector<std::uint8_t>>(wide)) == (std::vector<std::size_t>{0, 69}));

        // An empty one is an empty sequence.
        BOOST_CHECK(xstd::bit_convert<xstd::bit_vector>(boost::dynamic_bitset<>()).empty());
}

// Out: a sequence's size or a set's whole blocks, gathered into blocks of the target's own width.
BOOST_AUTO_TEST_CASE(AnOwnerConvertsOutKeepingItsWidthAndPositions)
{
        auto v = xstd::bit_vector(70);
        v[0]   = true;
        v[69]  = true;

        auto const same = xstd::bit_convert<boost::dynamic_bitset<std::size_t>>(v);
        BOOST_CHECK_EQUAL(same.size(), 70UZ);
        BOOST_CHECK(positions(same) == (std::vector<std::size_t>{0, 69}));
        BOOST_CHECK(positions(xstd::bit_convert<boost::dynamic_bitset<std::uint8_t>>(v)) == (std::vector<std::size_t>{0, 69}));

        // A narrower source block is gathered into a wider target one, as far as the source reaches.
        auto const octets = xstd::bit_convert<xstd::basic_bit_vector<std::uint8_t>>(make<std::uint8_t>(13, {12}));
        BOOST_CHECK(positions(xstd::bit_convert<boost::dynamic_bitset<std::uint64_t>>(octets)) == (std::vector<std::size_t>{12}));

        BOOST_CHECK_EQUAL(xstd::bit_convert<boost::dynamic_bitset<>>(xstd::bit_set{3, 64, 129}).size(), 192UZ);
        BOOST_CHECK(xstd::bit_convert<boost::dynamic_bitset<>>(xstd::bit_vector()).empty());
        BOOST_CHECK(positions(xstd::bit_convert<boost::dynamic_bitset<std::uint16_t>>(std::bitset<70>(0b101))) == (std::vector<std::size_t>{0, 2}));

        // And back: the round trip is the identity.
        auto const original = make<std::uint8_t>(13, {0, 5, 12});
        BOOST_CHECK(xstd::bit_convert<boost::dynamic_bitset<std::uint8_t>>(xstd::bit_convert<xstd::bit_vector>(original)) == original);
}

// A fixed target takes a dynamic_bitset by value: zero-extended, and std::overflow_error for a position past it.
BOOST_AUTO_TEST_CASE(AFixedTargetTakesADynamicBitsetByValue)
{
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint64_t>(make<std::uint8_t>(13, {0, 12})), (1ULL << 12U) | 1ULL);
        BOOST_CHECK(xstd::bit_convert<std::bitset<70>>(make<std::uint64_t>(70, {69})).test(69));
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<std::bitset<64>>(make<std::uint64_t>(70, {69}))), std::overflow_error);
        BOOST_CHECK(positions(xstd::bit_convert<boost::dynamic_bitset<std::uint8_t>>(xstd::bit_array<20>{true})) == (std::vector<std::size_t>{0}));
        BOOST_CHECK_EQUAL(xstd::bit_convert<boost::dynamic_bitset<std::uint8_t>>(xstd::bit_array<20>()).size(), 20UZ);
}

// Both directions are named by the one concept, a dynamic_bitset itself being no source for another.
BOOST_AUTO_TEST_CASE(BothDirectionsAreBitConvertible)
{
        static_assert(xstd::bit_convertible_to<boost::dynamic_bitset<>, xstd::bit_vector> and xstd::bit_convertible_to<xstd::bit_set, boost::dynamic_bitset<>>);
        static_assert(xstd::bit_convertible_to<boost::dynamic_bitset<>, std::uint64_t> and xstd::bit_convertible_to<std::bitset<70>, boost::dynamic_bitset<>>);
        static_assert(not xstd::bit_convertible_to<boost::dynamic_bitset<std::uint8_t>, boost::dynamic_bitset<>>);
        BOOST_CHECK(true);
}

// A bounded target that cannot hold the source throws what its own growth throws.
BOOST_AUTO_TEST_CASE(ABoundedTargetTooSmallThrowsWhatItsGrowthThrows)
{
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::bit_bounded_vector<100>>(boost::dynamic_bitset<>(101))), std::bad_alloc);

        // Blocks past a set's capacity drop, of the target's block width or not, and a key among them throws.
        BOOST_CHECK(positions(xstd::bit_convert<xstd::bit_bounded_set<100>>(make<std::size_t>(300, {5}))) == (std::vector<std::size_t>{5}));
        BOOST_CHECK(positions(xstd::bit_convert<xstd::bit_bounded_set<100>>(make<std::uint8_t>(300, {5}))) == (std::vector<std::size_t>{5}));
        BOOST_CHECK(positions(xstd::bit_convert<xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 100>>(make<std::uint8_t>(300, {5}))) == (std::vector<std::size_t>{5}));
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::bit_bounded_set<100>>(make<std::size_t>(300, {250}))), std::bad_alloc);
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::basic_bit_bounded_set<std::size_t, std::uint16_t, 100>>(make<std::uint8_t>(300, {250}))), std::bad_alloc);
}

// Boost hands none of its storage out, so an rvalue is copied from and keeps its bits.
BOOST_AUTO_TEST_CASE(NothingIsAdoptedFromADynamicBitset)
{
        auto b       = make<std::size_t>(70, {69});
        auto const v = xstd::bit_convert<xstd::bit_vector>(std::move(b));
        BOOST_CHECK(positions(v) == (std::vector<std::size_t>{69}));
        BOOST_CHECK_EQUAL(b.size(), 70UZ); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move): a copy leaves the source, which is the check.
}

#if BOOST_VERSION >= 109000

// Since Boost 1.90 the blocks may sit in a container of the caller's choosing, which converts like any other.
BOOST_AUTO_TEST_CASE(AContainerParameterConvertsAlike)
{
        using small_bitset = boost::dynamic_bitset<std::size_t, boost::container::small_vector<std::size_t, 2>>;
        auto b             = small_bitset(130);
        b.set(129);
        auto const v = xstd::bit_convert<xstd::bit_vector>(b);
        BOOST_CHECK_EQUAL(v.size(), 130UZ);
        BOOST_CHECK(xstd::bit_convert<small_bitset>(v) == b);
}

// Since Boost 1.90 a dynamic_bitset is a literal type, and both directions are constant expressions.
BOOST_AUTO_TEST_CASE(BothDirectionsAreConstantExpressions)
{
        static_assert([] -> bool {
                auto b = boost::dynamic_bitset<std::uint8_t>(13);
                b.set(12);
                auto const v    = xstd::bit_convert<xstd::bit_vector>(b);
                auto const back = xstd::bit_convert<boost::dynamic_bitset<std::uint16_t>>(v);
                return v.size() == 13UZ and v[12] and back.size() == 13UZ and back.test(12);
        }());
        BOOST_CHECK(true);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
