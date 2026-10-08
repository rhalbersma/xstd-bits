//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                     // wide_block_types
#include <test/for_each_type.hpp>                   // for_each_type
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/bit_fixed_set.hpp>              // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector
#include <xstd/bits/detail/hash.hpp>                // long_hash, short_hash, std_hash
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <xstd/bits/from_blocks.hpp>                // from_blocks
#include <boost/hash2/flavor.hpp>                   // default_flavor, little_endian_flavor
#include <boost/hash2/fnv1a.hpp>                    // fnv1a_32, fnv1a_64
#include <boost/hash2/hash_append.hpp>              // hash_append, hash_append_size
#include <boost/hash2/siphash.hpp>                  // siphash_64
#include <boost/hash2/xxhash.hpp>                   // xxhash_32, xxhash_64
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                    // array
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <functional>                               // hash
#include <initializer_list>                         // initializer_list
#include <vector>                                   // vector

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

// A hash that keeps the bytes it is given and counts the calls that gave them.
struct recorder
{
        std::vector<unsigned char> bytes;
        std::size_t updates = 0;

        auto update(void const* p, std::size_t n)
                -> void
        {
                auto const* const first = static_cast<unsigned char const*>(p);
                bytes.insert(bytes.end(), first, first + n);
                ++updates;
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

// std::hash of the object's own type, which decltype of a const object is not.
template<class T>
[[nodiscard]] auto default_digest(T const& v)
        -> std::size_t
{
        return std::hash<T>()(v);
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

BOOST_AUTO_TEST_CASE(TheDefaultIsShortHashUpToOneWordOfMessage)
{
        // One block of 64 bits is eight bytes, and two are sixteen, whatever the target.
        auto const one_word = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 64>({1, 3, 5});
        auto const two_word = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 65>({1, 3, 5});
        BOOST_CHECK_EQUAL(default_digest(one_word), xstd::bits::detail::std_hash(one_word, xstd::bits::detail::short_hash()));
        BOOST_CHECK_EQUAL(default_digest(two_word), xstd::bits::detail::std_hash(two_word, xstd::bits::detail::long_hash()));

        // A run-time width decides on the blocks it holds.
        auto const narrow = sequence(64);
        auto const wide   = sequence(65);
        BOOST_CHECK_EQUAL(default_digest(narrow), xstd::bits::detail::std_hash(narrow, xstd::bits::detail::short_hash()));
        BOOST_CHECK_EQUAL(default_digest(wide), xstd::bits::detail::std_hash(wide, xstd::bits::detail::long_hash()));

        // The positions take four bytes each under the default flavor, so two of them are a word and three are not.
        auto const two   = dynamic_set({1, 3});
        auto const three = dynamic_set({1, 3, 5});
        BOOST_CHECK_EQUAL(default_digest(two), xstd::bits::detail::std_hash(two, xstd::bits::detail::short_hash()));
        BOOST_CHECK_EQUAL(default_digest(three), xstd::bits::detail::std_hash(three, xstd::bits::detail::long_hash()));
}

// FNV-1a and xxHash, each at the width of the size_t that std::hash returns.
static_assert(std::same_as<xstd::bits::detail::short_hash, boost::hash2::fnv1a_64> or std::same_as<xstd::bits::detail::short_hash, boost::hash2::fnv1a_32>);
static_assert(std::same_as<xstd::bits::detail::long_hash, boost::hash2::xxhash_64> or std::same_as<xstd::bits::detail::long_hash, boost::hash2::xxhash_32>);
static_assert(sizeof(xstd::bits::detail::short_hash::result_type) == sizeof(std::size_t));
static_assert(sizeof(xstd::bits::detail::long_hash::result_type) == sizeof(std::size_t));

BOOST_AUTO_TEST_CASE(ASeededInstanceSubstitutes)
{
        auto const value    = make(0b1010'0101);
        constexpr auto seed = std::uint64_t{0x9E37'79B9'7F4A'7C15};

        // By value, not by type alone: a seed is state the type cannot carry.
        BOOST_CHECK_EQUAL(
                xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_64(seed)),
                xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_64(seed))
        );
        BOOST_CHECK(xstd::bits::detail::std_hash(value, boost::hash2::fnv1a_64(seed)) != default_digest(value));
}

BOOST_AUTO_TEST_CASE(AnotherAlgorithmSubstitutes)
{
        auto const value = make(0b1010'0101);
        BOOST_CHECK(xstd::bits::detail::std_hash(value, boost::hash2::xxhash_64()) != default_digest(value));
        BOOST_CHECK(xstd::bits::detail::std_hash(value, boost::hash2::siphash_64()) != default_digest(value));
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

// The blocks go in as one range, in the bytes that appending them one at a time writes.
BOOST_AUTO_TEST_CASE(TheBlocksGoInAsOneRange)
{
        auto const value = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 200>({1, 64, 199});
        BOOST_CHECK_EQUAL(record<boost::hash2::default_flavor>(value).updates, 2UZ);

        auto one_by_one = recorder();
        for (auto const block : {std::uint64_t{2}, std::uint64_t{1}, std::uint64_t{0}, std::uint64_t{1} << 7U}) {
                boost::hash2::hash_append(one_by_one, boost::hash2::little_endian_flavor(), block);
        }
        boost::hash2::hash_append_size(one_by_one, boost::hash2::little_endian_flavor(), 200UZ);
        BOOST_CHECK(record<boost::hash2::little_endian_flavor>(value).bytes == one_by_one.bytes);
}

// A block wider than Hash2 writes goes in as its 64-bit words, low first: the message 64-bit blocks of its bits give.
BOOST_AUTO_TEST_CASE(AWideBlockPinsTheDigestOfItsWords)
{
        auto const narrow = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 200>({1, 64, 199});
        BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::fnv1a_64>(narrow), 0x4DCE'2DB6'6171'FAEEULL);
        BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::xxhash_64>(narrow), 0xA308'EADD'798A'589FULL);
        test::for_each_type<test::wide_block_types>([&]<class Block> -> void {
                auto const wide = xstd::basic_bit_fixed_set<std::size_t, Block, 200>({1, 64, 199});
                BOOST_CHECK(record<boost::hash2::little_endian_flavor>(wide).bytes == record<boost::hash2::little_endian_flavor>(narrow).bytes);
                BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::fnv1a_64>(wide), 0x4DCE'2DB6'6171'FAEEULL);
                BOOST_CHECK_EQUAL(little_endian_digest<boost::hash2::xxhash_64>(wide), 0xA308'EADD'798A'589FULL);
        });
}

// Equal values hash equal under the default whatever holds them, at a short message and at a long one.
BOOST_AUTO_TEST_CASE(EqualValuesHashEqualUnderTheDefaultAcrossStorages)
{
        using block_type = std::uint64_t;

        // The set reading at a static width: an owner, a view over it, and a view over borrowed blocks.
        auto const fixed_set_digests = []<std::size_t N, std::size_t K>(std::initializer_list<std::size_t> keys, std::array<block_type, K> const& blocks) -> std::array<std::size_t, 3> {
                auto const owned = xstd::basic_bit_fixed_set<std::size_t, block_type, N>(keys);
                auto const view  = xstd::bit_set_view(owned);
                auto const lent  = xstd::bit_set_view(blocks);
                return {default_digest(owned), default_digest(view), default_digest(lent)};
        };
        auto const short_fixed = fixed_set_digests.operator()<64>({1, 3, 63}, std::array<block_type, 1>{0x8000'0000'0000'000AULL});
        auto const long_fixed  = fixed_set_digests.operator()<128>({1, 3, 127}, std::array<block_type, 2>{0xAULL, 0x8000'0000'0000'0000ULL});
        BOOST_CHECK(short_fixed[0] == short_fixed[1] and short_fixed[1] == short_fixed[2]);
        BOOST_CHECK(long_fixed[0] == long_fixed[1] and long_fixed[1] == long_fixed[2]);

        // The set reading at a run-time width: every resizable owner, and a view over one.
        auto const dynamic_set_digests = [](std::initializer_list<std::size_t> keys) -> std::array<std::size_t, 4> {
                auto const growing      = xstd::basic_bit_set<std::size_t, block_type>(keys);
                auto const bounded      = xstd::basic_bit_bounded_set<std::size_t, block_type, 256>(keys);
                auto const inline_first = xstd::basic_bit_small_set<std::size_t, block_type, 256>(keys);
                auto const view         = xstd::bit_set_view(growing);
                return {default_digest(growing), default_digest(bounded), default_digest(inline_first), default_digest(view)};
        };
        for (auto const& digests : {dynamic_set_digests({1, 3}), dynamic_set_digests({1, 3, 5, 200})}) {
                BOOST_CHECK(digests[0] == digests[1] and digests[1] == digests[2] and digests[2] == digests[3]);
        }

        // The sequence reading, every owner: a view of it hashes no more than std::span does.
        auto const sequence_digests = []<std::size_t N> -> std::array<std::size_t, 4> {
                auto const fixed        = alternating(xstd::basic_bit_array<block_type, N>());
                auto const growing      = alternating(xstd::basic_bit_vector<block_type>(N));
                auto const bounded      = alternating(xstd::basic_bit_bounded_vector<block_type, 256>(N));
                auto const inline_first = alternating(xstd::basic_bit_small_vector<block_type, 256>(N));
                return {default_digest(fixed), default_digest(growing), default_digest(bounded), default_digest(inline_first)};
        };
        for (auto const& digests : {sequence_digests.operator()<10>(), sequence_digests.operator()<200>()}) {
                BOOST_CHECK(digests[0] == digests[1] and digests[1] == digests[2] and digests[2] == digests[3]);
        }
}

BOOST_AUTO_TEST_SUITE_END()
