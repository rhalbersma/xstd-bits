//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                    // all_block_types
#include <test/for_each_type.hpp>                  // for_each_type
#include <xstd/bits/bit_functional/bit_digest.hpp> // bit_digest
#include <xstd/bits/bit_functional/bit_hash.hpp>   // bit_hash, bit_hash_append
#include <xstd/bits/bit_set/bit_fixed_set.hpp>     // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>           // basic_bit_set
#include <boost/hash2/flavor.hpp>                  // default_flavor, little_endian_flavor
#include <boost/hash2/fnv1a.hpp>                   // fnv1a_64
#include <boost/hash2/get_integral_result.hpp>     // get_integral_result
#include <boost/hash2/xxh3.hpp>                    // xxh3_128
#include <boost/hash2/xxhash.hpp>                  // xxhash_64
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                                // same_as
#include <cstddef>                                 // size_t
#include <cstdint>                                 // uint64_t, uint8_t
#include <tuple>                                   // tuple

BOOST_AUTO_TEST_SUITE(BitDigest)

namespace {

template<class F>
concept transparent = requires { typename F::is_transparent; };

} // namespace

// XXH3-128 over little-endian bytes by default: the algorithm's whole result, the same on every target.
BOOST_AUTO_TEST_CASE(ItDefaultsToXxh3At128BitsOverLittleEndianBytes)
{
        auto const value = xstd::bit_fixed_set<100>({1, 3, 99});
        auto h           = boost::hash2::xxh3_128();
        xstd::bit_hash_append(h, boost::hash2::little_endian_flavor(), value);
        BOOST_CHECK(xstd::bit_digest<xstd::bit_fixed_set<100>>()(value) == h.result());
        static_assert(std::same_as<xstd::bit_digest<xstd::bit_fixed_set<100>>::result_type, boost::hash2::xxh3_128::result_type>);
        static_assert(not transparent<xstd::bit_digest<xstd::bit_fixed_set<100>>>);
}

// The digests pinned at every block type: a block's width is no part of the string, so equal bits digest equal.
BOOST_AUTO_TEST_CASE(EqualBitsDigestEqualAcrossBlockTypes)
{
        test::for_each_type<test::all_block_types>([]<class Block> -> void {
                using X          = xstd::basic_bit_fixed_set<std::size_t, Block, 200>;
                auto const fixed = X({1, 64, 199});
                BOOST_CHECK_EQUAL((xstd::bit_digest<X, boost::hash2::fnv1a_64>()(fixed)), 0xB72B'9E62'AF48'F732ULL);
                BOOST_CHECK_EQUAL((xstd::bit_digest<X, boost::hash2::xxhash_64>()(fixed)), 0xF8C0'7F80'B85C'9EA1ULL);
        });
        test::for_each_type<std::tuple<std::uint8_t, std::uint64_t>>([]<class Block> -> void {
                using X = xstd::basic_bit_set<std::size_t, Block>;
                BOOST_CHECK_EQUAL((xstd::bit_digest<X, boost::hash2::fnv1a_64>()(X({1, 64, 199}))), 0x4EAC'991A'7BBE'935BULL);
        });
}

// The hash keeps a std::size_t's worth of the digest its own algorithm and flavor make of the same bits.
BOOST_AUTO_TEST_CASE(TheHashFoldsTheDigestOfItsOwnAlgorithmAndFlavor)
{
        using X          = xstd::bit_fixed_set<100>;
        auto const value = X({1, 3, 99});
        auto h           = boost::hash2::xxhash_64();
        xstd::bit_hash_append(h, boost::hash2::default_flavor(), value);
        auto folded = h;
        BOOST_CHECK_EQUAL((xstd::bit_digest<X, boost::hash2::xxhash_64, boost::hash2::default_flavor>()(value)), h.result());
        BOOST_CHECK_EQUAL(xstd::bit_hash<X>()(value), boost::hash2::get_integral_result<std::size_t>(folded));
}

BOOST_AUTO_TEST_SUITE_END()
