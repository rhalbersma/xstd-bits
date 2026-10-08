//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_fixed_set.hpp> // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set.hpp>       // basic_bit_set
#include <xstd/bits/bit_vector.hpp>    // basic_bit_vector
#include <xstd/bits/detail/hash.hpp>   // std_hash
#include <xstd/bits/from_blocks.hpp>   // from_blocks
#include <boost/hash2/flavor.hpp>      // little_endian_flavor
#include <boost/hash2/fnv1a.hpp>       // fnv1a_32, fnv1a_64
#include <boost/hash2/hash_append.hpp> // hash_append
#include <boost/hash2/xxhash.hpp>      // xxhash_64
#include <boost/test/unit_test.hpp>    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <cstddef>                     // size_t
#include <cstdint>                     // uint64_t, uint8_t
#include <functional>                  // hash
#include <vector>                      // vector

// std_hash's Hash parameter is the one thing std::hash cannot reach, so it is asserted here.
BOOST_AUTO_TEST_SUITE(DetailHash)

namespace {

using set_type = xstd::bit_fixed_set<8>;

// The positions an integer's set bits name, through the byte exchange.
[[nodiscard]] auto make(std::uint8_t bits)
        -> set_type
{
        return {xstd::from_blocks, bits};
}

// Blocks of a width fixed on every target, so that a message's bytes depend on the flavor alone.
using fixed_set   = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 10>;
using dynamic_set = xstd::basic_bit_set<std::size_t, std::uint64_t>;
using sequence    = xstd::basic_bit_vector<std::uint64_t>;

// A hash that keeps the bytes it is given.
struct recorder
{
        std::vector<unsigned char> bytes;

        auto update(void const* p, std::size_t n)
                -> void
        {
                auto const* const first = static_cast<unsigned char const*>(p);
                bytes.insert(bytes.end(), first, first + n);
        }
};

template<class Flavor, class T>
[[nodiscard]] auto record(T const& v)
        -> recorder
{
        auto h = recorder();
        boost::hash2::hash_append(h, Flavor(), v);
        return h;
}

template<class Hash, class T>
[[nodiscard]] constexpr auto little_endian_digest(T const& v)
        -> Hash::result_type
{
        auto h = Hash();
        boost::hash2::hash_append(h, boost::hash2::little_endian_flavor(), v);
        return h.result();
}

// Every other position set, from the first on, through the subscript every owner of the sequence reading has.
template<class T>
[[nodiscard]] auto alternating(T x)
        -> T
{
        for (auto i = 0UZ; i < x.size(); i += 2UZ) {
                x[i] = true;
        }
        return x;
}

} // namespace

BOOST_AUTO_TEST_CASE(TheDefaultIsFnv1a64)
{
        auto const value = make(0b1010'0101);
        BOOST_CHECK_EQUAL(xstd::bits::detail::std_hash(value), xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_64()));

        // And it is the default std::hash takes, since its operator() has no second argument to pass.
        BOOST_CHECK_EQUAL(std::hash<set_type>()(value), xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_64()));
}

BOOST_AUTO_TEST_CASE(ASeededInstanceSubstitutes)
{
        auto const value    = make(0b1010'0101);
        constexpr auto seed = std::uint64_t{0x9E37'79B9'7F4A'7C15};

        // By value, not by type alone: this is what a defaulted template parameter on its own could not express.
        BOOST_CHECK_EQUAL(
                xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_64(seed)),
                xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_64(seed))
        );
        BOOST_CHECK(xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_64(seed)) != xstd::bits::detail::std_hash(value));
}

BOOST_AUTO_TEST_CASE(AnotherAlgorithmSubstitutes)
{
        auto const value = make(0b1010'0101);
        BOOST_CHECK(xstd::bits::detail::std_hash(value, boost::hash2::xxhash_64()) != xstd::bits::detail::std_hash(value));
        BOOST_CHECK(xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_32()) != xstd::bits::detail::std_hash(value));
}

// The invariant is per algorithm, and holds under a substituted one too: equal values hash equal.
BOOST_AUTO_TEST_CASE(EqualValuesHashEqualUnderASubstitutedHash)
{
        auto const lhs   = make(0b1010'0101);
        auto const rhs   = make(0b1010'0101);
        auto const other = make(0b0101'1010);

        BOOST_CHECK_EQUAL(
                xstd::bits::detail::std_hash(lhs, boost::hash2::xxhash_64()),
                xstd::bits::detail::std_hash(rhs, boost::hash2::xxhash_64())
        );
        BOOST_CHECK(
                xstd::bits::detail::std_hash(lhs, boost::hash2::xxhash_64()) !=
                xstd::bits::detail::std_hash(other, boost::hash2::xxhash_64())
        );
}

// A size and a position are the flavor's size_type, so a fixed flavor fixes every byte on every target.
BOOST_AUTO_TEST_CASE(AFixedFlavorFixesTheMessage)
{
        auto const fixed = record<boost::hash2::little_endian_flavor>(fixed_set({1, 3, 5}));
        BOOST_CHECK((fixed.bytes == std::vector<unsigned char>{0x2A, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0}));

        auto const dynamic = record<boost::hash2::little_endian_flavor>(dynamic_set({1, 3, 5}));
        BOOST_CHECK((dynamic.bytes == std::vector<unsigned char>{1, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0}));

        auto const bits = record<boost::hash2::little_endian_flavor>(alternating(sequence(6)));
        BOOST_CHECK((bits.bytes == std::vector<unsigned char>{0x15, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0}));
}

// The digests those messages give, the same on a 32-bit target as on a 64-bit one.
BOOST_AUTO_TEST_CASE(AFixedFlavorPinsTheDigest)
{
        static_assert(little_endian_digest<boost::hash2::fnv1a_64>(fixed_set({1, 3, 5})) == 0x00CF'A8BE'F13E'27B5ULL);
        BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::fnv1a_64>(fixed_set({1, 3, 5})), 0x00CF'A8BE'F13E'27B5ULL);
        BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::xxhash_64>(fixed_set({1, 3, 5})), 0x66E3'375D'2854'268AULL);
        BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::fnv1a_64>(dynamic_set({1, 3, 5})), 0xDE0C'059C'D172'C701ULL);
        BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::xxhash_64>(dynamic_set({1, 3, 5})), 0x1502'1EF5'D5C1'3C98ULL);
        BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::fnv1a_64>(alternating(sequence(6))), 0x9E2E'60F0'092F'3F96ULL);
        BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::xxhash_64>(alternating(sequence(6))), 0x2C1F'1D26'6824'9D6EULL);
}

BOOST_AUTO_TEST_SUITE_END()
