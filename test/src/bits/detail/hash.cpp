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
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set, bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <xstd/bits/from_blocks.hpp>                // from_blocks
#include <xstd/misc/ext/boost/hash2.hpp>            // hash, long_hash, short_hash
#include <boost/hash2/flavor.hpp>                   // default_flavor, little_endian_flavor
#include <boost/hash2/fnv1a.hpp>                    // fnv1a_64
#include <boost/hash2/get_integral_result.hpp>      // get_integral_result
#include <boost/hash2/hash_append.hpp>              // hash_append, hash_append_size
#include <boost/hash2/siphash.hpp>                  // siphash_64
#include <boost/hash2/xxhash.hpp>                   // xxhash_64
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <boost/unordered/unordered_flat_set.hpp>   // unordered_flat_set
#include <array>                                    // array
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <functional>                               // hash
#include <initializer_list>                         // initializer_list
#include <unordered_set>                            // unordered_set
#include <vector>                                   // vector

// The messages the hooks append, the algorithm std::hash picks for them, and xstd::hash running any other.
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

// A seed of the kind a container draws at random, fixed here so that a failure reruns.
constexpr auto test_seed = std::uint64_t{0x9E37'79B9'7F4A'7C15};

// The public hasher, unseeded or seeded, named rather than a temporary.
template<class Hash, class T, class... Seed>
[[nodiscard]] auto xstd_digest(T const& v, Seed... seeds)
        -> std::size_t
{
        auto const hasher = xstd::hash<T, Hash>(seeds...);
        return hasher(v);
}

// What Hash2 makes of the value under an algorithm instance and the default flavor, folded as std::hash is.
template<class Hash, class T>
[[nodiscard]] auto algorithm_digest(Hash h, T const& v)
        -> std::size_t
{
        boost::hash2::hash_append(h, boost::hash2::default_flavor(), v);
        return boost::hash2::get_integral_result<std::size_t>(h);
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
        BOOST_CHECK_EQUAL(default_digest(one_word), xstd_digest<xstd::short_hash>(one_word));
        BOOST_CHECK_EQUAL(default_digest(two_word), xstd_digest<xstd::long_hash>(two_word));

        // A run-time width decides on the blocks it holds.
        auto const narrow = sequence(64);
        auto const wide   = sequence(65);
        BOOST_CHECK_EQUAL(default_digest(narrow), xstd_digest<xstd::short_hash>(narrow));
        BOOST_CHECK_EQUAL(default_digest(wide), xstd_digest<xstd::long_hash>(wide));

        // The positions take four bytes each under the default flavor, so two of them are a word and three are not.
        auto const two   = dynamic_set({1, 3});
        auto const three = dynamic_set({1, 3, 5});
        BOOST_CHECK_EQUAL(default_digest(two), xstd_digest<xstd::short_hash>(two));
        BOOST_CHECK_EQUAL(default_digest(three), xstd_digest<xstd::long_hash>(three));
}

BOOST_AUTO_TEST_CASE(ASeededHasherDiffersFromTheDefault)
{
        auto const value    = make(0b1010'0101);
        auto const seeded   = xstd_digest<boost::hash2::fnv1a_64>(value, test_seed);
        auto const reseeded = xstd_digest<boost::hash2::fnv1a_64>(value, test_seed);
        auto const unseeded = xstd_digest<boost::hash2::fnv1a_64>(value);
        BOOST_CHECK_EQUAL(seeded, reseeded);
        BOOST_CHECK(seeded != unseeded);
        BOOST_CHECK(seeded != default_digest(value));
}

BOOST_AUTO_TEST_CASE(AnotherAlgorithmDiffersFromTheDefault)
{
        auto const value = make(0b1010'0101);
        BOOST_CHECK(xstd_digest<boost::hash2::xxhash_64>(value) != default_digest(value));
        BOOST_CHECK(xstd_digest<boost::hash2::siphash_64>(value) != default_digest(value));
}

// The invariant is per algorithm, and holds under a chosen one too: equal values hash equal.
BOOST_AUTO_TEST_CASE(EqualValuesHashEqualUnderAChosenAlgorithm)
{
        auto const lhs   = make(0b1010'0101);
        auto const rhs   = make(0b1010'0101);
        auto const other = make(0b0101'1010);
        BOOST_CHECK_EQUAL(xstd_digest<boost::hash2::xxhash_64>(lhs), xstd_digest<boost::hash2::xxhash_64>(rhs));
        BOOST_CHECK(xstd_digest<boost::hash2::xxhash_64>(lhs) != xstd_digest<boost::hash2::xxhash_64>(other));
}

// Every owner and view hashes through xstd::hash as Hash2 hashes it, under the algorithm and the seed it holds.
BOOST_AUTO_TEST_CASE(XstdHashRunsTheAlgorithmItIsGiven)
{
        auto const check = []<class T>(T const& v) -> void {
                BOOST_CHECK_EQUAL(xstd_digest<boost::hash2::siphash_64>(v), algorithm_digest(boost::hash2::siphash_64(), v));
                BOOST_CHECK_EQUAL(xstd_digest<boost::hash2::siphash_64>(v, test_seed), algorithm_digest(boost::hash2::siphash_64(test_seed), v));
                BOOST_CHECK_EQUAL(xstd_digest<boost::hash2::xxhash_64>(v, test_seed), algorithm_digest(boost::hash2::xxhash_64(test_seed), v));
        };
        auto const growing = xstd::bit_set({1, 3, 200});
        check(xstd::bit_fixed_set<100>({1, 3, 99}));
        check(growing);
        check(alternating(xstd::bit_vector(130)));
        check(xstd::bit_set_view(growing));
}

// Untrusted keys, as the documentation advises: SipHash, seeded per container, in either kind of table.
BOOST_AUTO_TEST_CASE(ASeededSipHashKeysAnUnorderedContainer)
{
        auto const fill = []<class T, template<class...> class Table>(std::initializer_list<T> values) -> void {
                using hasher = xstd::hash<T, boost::hash2::siphash_64>;
                for (auto const& h : {hasher(), hasher(test_seed)}) {
                        auto table = Table<T, hasher>(0, h);
                        for (auto const& v : values) {
                                table.insert(v);
                        }
                        BOOST_CHECK_EQUAL(table.size(), 2UZ);
                        for (auto const& v : values) {
                                BOOST_CHECK(table.contains(v));
                        }
                }
        };
        auto const fill_both = [&]<class T>(std::initializer_list<T> values) -> void {
                fill.template operator()<T, std::unordered_set>(values);
                fill.template operator()<T, boost::unordered_flat_set>(values);
        };

        // Three values, two of them equal, so each table holds two.
        fill_both.operator()<xstd::bit_fixed_set<100>>({xstd::bit_fixed_set<100>({1, 99}), xstd::bit_fixed_set<100>({1, 99}), xstd::bit_fixed_set<100>({2})});
        fill_both.operator()<xstd::bit_set>({xstd::bit_set({1, 200}), xstd::bit_set({1, 200}), xstd::bit_set({2})});
        fill_both.operator()<xstd::bit_vector>({alternating(xstd::bit_vector(130)), alternating(xstd::bit_vector(130)), xstd::bit_vector(130)});

        // A view keys the table as its owner's value, so a second view of equal bits is found, not added.
        auto const first  = xstd::bit_set({1, 200});
        auto const second = xstd::bit_set({1, 200});
        auto const third  = xstd::bit_set({2});
        using view_type   = decltype(xstd::bit_set_view(first));
        fill_both.operator()<view_type>({xstd::bit_set_view(first), xstd::bit_set_view(second), xstd::bit_set_view(third)});
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
