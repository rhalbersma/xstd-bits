//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                     // all_block_types
#include <test/for_each_type.hpp>                   // for_each_type
#include <xstd/bits/bit_array.hpp>                  // bit_array
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/bit_fixed_set.hpp>              // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_hasher.hpp>                 // bit_hash_append, bit_hasher
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set, bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <xstd/bits/from_blocks.hpp>                // from_blocks
#include <xstd/misc/ext/boost/hash2.hpp>            // hash_algorithm, hasher
#include <boost/hash2/flavor.hpp>                   // default_flavor, little_endian_flavor
#include <boost/hash2/fnv1a.hpp>                    // fnv1a_64
#include <boost/hash2/get_integral_result.hpp>      // get_integral_result
#include <boost/hash2/hash_append.hpp>              // hash_append, hash_append_size
#include <boost/hash2/hash_append_fwd.hpp>          // hash_append_tag
#include <boost/hash2/siphash.hpp>                  // siphash_64
#include <boost/hash2/xxhash.hpp>                   // xxhash_64
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                    // array
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <functional>                               // hash
#include <initializer_list>                         // initializer_list
#include <ranges>                                   // iota
#include <span>                                     // span
#include <tuple>                                    // tuple
#include <type_traits>                              // is_nothrow_default_constructible_v
#include <vector>                                   // vector

// The bits as bytes: one string whatever blocks hold it, one update wherever the storage is that string.
BOOST_AUTO_TEST_SUITE(BitHasher)

namespace {

using byte_string = std::vector<unsigned char>;

// An algorithm that keeps the bytes it is given and counts the calls that gave them.
struct recorder
{
        using result_type = std::uint64_t;

        byte_string message;
        std::size_t updates = 0;

        [[nodiscard]] recorder() = default;

        [[nodiscard]] explicit recorder(std::uint64_t /* seed */) {}

        [[nodiscard]] recorder(unsigned char const* /* seed */, std::size_t /* size */) {}

        auto update(void const* p, std::size_t n)
                -> void
        {
                auto const* const first = static_cast<unsigned char const*>(p);
                message.insert(message.end(), first, first + n);
                ++updates;
        }

        [[nodiscard]] auto result() const
                -> result_type
        {
                return message.size();
        }
};

static_assert(xstd::hash_algorithm<recorder>);

// An algorithm H that also counts its updates, which its digest does not see.
template<class H>
struct counting : H
{
        std::size_t updates = 0;

        using H::H;

        auto update(void const* p, std::size_t n)
                -> void
        {
                H::update(p, n);
                ++updates;
        }
};

static_assert(xstd::hash_algorithm<counting<boost::hash2::fnv1a_64>>);

// The message under a flavor that fixes the bytes of a size on every target.
template<class T>
[[nodiscard]] auto record(T const& x)
        -> recorder
{
        auto h = recorder();
        xstd::bit_hash_append(h, boost::hash2::little_endian_flavor(), x);
        return h;
}

template<class Hash, class T>
[[nodiscard]] constexpr auto bit_digest(T const& x)
        -> Hash::result_type
{
        auto h = Hash();
        xstd::bit_hash_append(h, boost::hash2::little_endian_flavor(), x);
        return h.result();
}

// One update over a byte string, and what it makes of it.
template<class Hash>
[[nodiscard]] auto string_hash(byte_string const& message)
        -> Hash
{
        auto h = Hash();
        h.update(message.data(), message.size());
        return h;
}

template<class Hash>
[[nodiscard]] auto string_digest(byte_string const& message)
        -> Hash::result_type
{
        auto h = string_hash<Hash>(message);
        return h.result();
}

template<class Hash>
[[nodiscard]] auto string_digest(byte_string const& message, std::size_t size)
        -> Hash::result_type
{
        auto h = Hash();
        h.update(message.data(), message.size());
        boost::hash2::hash_append_size(h, boost::hash2::little_endian_flavor(), size);
        return h.result();
}

// Blocks of one width on every target, so that a message depends on the flavor alone.
using fixed_set   = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 10>;
using dynamic_set = xstd::basic_bit_set<std::size_t, std::uint64_t>;
using sequence    = xstd::basic_bit_vector<std::uint64_t>;

// Every other position set, from the first on.
template<class T>
[[nodiscard]] constexpr auto alternating(T x)
        -> T
{
        for (auto i = 0UZ; i < x.size(); i += 2UZ) {
                x[i] = true;
        }
        return x;
}

// A seed of the kind a container draws at random, fixed here so that a failure reruns.
constexpr auto test_seed = std::uint64_t{0x9E37'79B9'7F4A'7C15};

// The positions a long set holds: past the first block, across 256 bytes and up to its last position.
inline constexpr auto long_width = 4'000UZ;
inline constexpr auto long_keys  = {1UZ, 64UZ, 199UZ, 2'047UZ, 2'048UZ, 3'999UZ};

template<class Block>
using long_set = xstd::basic_bit_fixed_set<std::size_t, Block, long_width>;

// Six kinds of piece a side, each kind a plane of squares, as a board game's position holds them.
inline constexpr auto kinds = 6UZ;
inline constexpr auto sides = 2UZ;

template<std::size_t N>
using plane = xstd::bit_fixed_set<N>;

// A plane per kind and side, its blocks a pattern that differs between every two of them.
template<std::size_t N>
[[nodiscard]] constexpr auto plane_blocks(std::size_t k) noexcept
        -> std::uint64_t
{
        auto const blocks = (k + 1UZ) * std::uint64_t{0x0123'4567'89AB'CDEF};
        if constexpr (N < 64UZ) {
                return blocks & ((std::uint64_t{1} << N) - 1UZ);
        } else {
                return blocks;
        }
}

template<std::size_t N>
[[nodiscard]] constexpr auto make_plane(std::size_t k)
        -> plane<N>
{
        return {xstd::from_blocks, plane_blocks<N>(k)};
}

// The string a plane is: its ceil(N / 8) low bytes, least significant first.
template<std::size_t N>
auto append_string(byte_string& message, std::size_t k)
        -> void
{
        auto const blocks = plane_blocks<N>(k);
        for (auto const j : std::views::iota(0UZ, (N + 7UZ) / 8UZ)) {
                message.push_back(static_cast<unsigned char>(blocks >> (8UZ * j)));
        }
}

template<std::size_t N>
[[nodiscard]] auto planes_string(std::size_t count)
        -> byte_string
{
        auto message = byte_string();
        for (auto const k : std::views::iota(0UZ, count)) {
                append_string<N>(message, k);
        }
        return message;
}

// A position whose own hook appends each side's planes in turn, as an aggregate composes them.
template<std::size_t N>
struct position
{
        plane<N> black[kinds]; // NOLINT(modernize-avoid-c-arrays): the aggregate a user writes
        plane<N> white[kinds]; // NOLINT(modernize-avoid-c-arrays): the aggregate a user writes

        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, position const* p)
                -> void
        {
                xstd::bit_hash_append(h, f, p->black);
                xstd::bit_hash_append(h, f, p->white);
        }
};

template<std::size_t N>
[[nodiscard]] constexpr auto make_position()
        -> position<N>
{
        auto p = position<N>();
        for (auto const k : std::views::iota(0UZ, kinds)) {
                p.black[k] = make_plane<N>(k);
                p.white[k] = make_plane<N>(kinds + k);
        }
        return p;
}

template<class Hash, class T>
[[nodiscard]] auto counted(T const& x)
        -> counting<Hash>
{
        auto h = counting<Hash>();
        boost::hash2::hash_append(h, boost::hash2::little_endian_flavor(), x);
        return h;
}

template<class Hash, class T>
[[nodiscard]] auto counted_bits(T const& x)
        -> counting<Hash>
{
        auto h = counting<Hash>();
        xstd::bit_hash_append(h, boost::hash2::little_endian_flavor(), x);
        return h;
}

} // namespace

BOOST_AUTO_TEST_CASE(TheMessageIsTheCanonicalByteString)
{
        // A static width appends ceil(N / 8) bytes and no size, as std::array appends none.
        BOOST_CHECK((record(fixed_set({1, 3, 5, 9})).message == byte_string{0x2A, 0x02}));

        // Empty, the one byte Hash2 has every append write.
        BOOST_CHECK((record(xstd::bit_fixed_set<0>()).message == byte_string{0x00}));
        BOOST_CHECK((record(xstd::bit_array<0>()).message == byte_string{0x00}));

        // A sequence of run-time width: its bytes, then its width in bits.
        BOOST_CHECK((record(alternating(sequence(10))).message == byte_string{0x55, 0x01, 10, 0, 0, 0}));
        BOOST_CHECK((record(sequence()).message == byte_string{0, 0, 0, 0}));

        // A set of run-time width: its bytes up to the last key, then their count.
        BOOST_CHECK((record(dynamic_set({1, 3, 5})).message == byte_string{0x2A, 1, 0, 0, 0}));
        BOOST_CHECK((record(dynamic_set({1, 3, 64})).message == byte_string{0x0A, 0, 0, 0, 0, 0, 0, 0, 0x01, 9, 0, 0, 0}));
        BOOST_CHECK((record(dynamic_set()).message == byte_string{0, 0, 0, 0}));
}

// Equal sets need not share a width, so the bytes past the last key are no part of the message.
BOOST_AUTO_TEST_CASE(ARunTimeSetHashesEqualAtEveryWidth)
{
        auto const narrow = xstd::bit_set({1, 3});
        auto wide         = xstd::bit_set({1, 3, 500});
        wide.erase(500);
        BOOST_CHECK(narrow == wide);
        BOOST_CHECK(record(narrow).message == record(wide).message);

        auto const hasher = xstd::bit_hasher<boost::hash2::xxhash_64>();
        BOOST_CHECK_EQUAL(hasher(xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 17>({1, 3})), hasher(narrow));
        BOOST_CHECK_EQUAL(hasher(xstd::basic_bit_bounded_set<std::size_t, std::uint64_t, 4'097>({1, 3})), hasher(narrow));
        BOOST_CHECK_EQUAL(hasher(xstd::basic_bit_small_set<std::size_t, std::uint8_t, 9>({1, 3})), hasher(narrow));
        BOOST_CHECK_EQUAL(hasher(xstd::bit_set_view(wide)), hasher(narrow));
}

// The digests pinned at every block type: a block's width is no part of the string.
BOOST_AUTO_TEST_CASE(EqualBitsHashEqualAcrossBlockTypes)
{
        test::for_each_type<test::all_block_types>([]<class Block> -> void {
                auto const fixed = xstd::basic_bit_fixed_set<std::size_t, Block, 200>({1, 64, 199});
                BOOST_CHECK_EQUAL(bit_digest<boost::hash2::fnv1a_64>(fixed), 0xB72B'9E62'AF48'F732ULL);
                BOOST_CHECK_EQUAL(bit_digest<boost::hash2::xxhash_64>(fixed), 0xF8C0'7F80'B85C'9EA1ULL);

                // More than one buffer's worth where the bytes are assembled.
                auto const long_value = long_set<Block>(long_keys);
                BOOST_CHECK_EQUAL(bit_digest<boost::hash2::xxhash_64>(long_value), 0x92BD'8F15'09F3'48A8ULL);
                BOOST_CHECK((record(long_value).message == record(long_set<std::uint8_t>(long_keys)).message));
        });

        // A run-time width at the narrowest and the widest word-sized block.
        test::for_each_type<std::tuple<std::uint8_t, std::uint64_t>>([]<class Block> -> void {
                auto const growing = xstd::basic_bit_set<std::size_t, Block>({1, 64, 199});
                BOOST_CHECK_EQUAL(bit_digest<boost::hash2::fnv1a_64>(growing), 0x4EAC'991A'7BBE'935BULL);

                auto const bools = alternating(xstd::basic_bit_vector<Block>(130));
                BOOST_CHECK_EQUAL(bit_digest<boost::hash2::fnv1a_64>(bools), 0x6F6F'BA15'A239'791EULL);
        });
}

// A constant expression assembles the string a byte at a time, and gets the one update over the storage gives.
BOOST_AUTO_TEST_CASE(TheAssembledStringIsTheStorage)
{
        constexpr auto fixed = bit_digest<boost::hash2::fnv1a_64>(long_set<std::uint64_t>(long_keys));
        BOOST_CHECK_EQUAL(bit_digest<boost::hash2::fnv1a_64>(long_set<std::uint64_t>(long_keys)), fixed);

        constexpr auto growing = bit_digest<boost::hash2::fnv1a_64>(dynamic_set({1, 64, 199}));
        BOOST_CHECK_EQUAL(bit_digest<boost::hash2::fnv1a_64>(dynamic_set({1, 64, 199})), growing);

        constexpr auto bools = bit_digest<boost::hash2::fnv1a_64>(alternating(sequence(130)));
        BOOST_CHECK_EQUAL(bit_digest<boost::hash2::fnv1a_64>(alternating(sequence(130))), bools);

        constexpr auto empty = bit_digest<boost::hash2::fnv1a_64>(xstd::bit_fixed_set<0>());
        BOOST_CHECK_EQUAL(bit_digest<boost::hash2::fnv1a_64>(xstd::bit_fixed_set<0>()), empty);
}

// Where the storage is the string, one update takes it all, and only a run-time width adds a second.
BOOST_AUTO_TEST_CASE(TheStorageGoesInAsOneUpdate)
{
        BOOST_CHECK_EQUAL(counted_bits<boost::hash2::fnv1a_64>(long_set<std::uint64_t>(long_keys)).updates, 1UZ);
        BOOST_CHECK_EQUAL(counted_bits<boost::hash2::fnv1a_64>(alternating(sequence(130))).updates, 2UZ);
        BOOST_CHECK_EQUAL(counted_bits<boost::hash2::fnv1a_64>(dynamic_set({1, 64, 199})).updates, 2UZ);
}

// Planes that fill their blocks are their strings back to back, so the array of them goes in as one update.
BOOST_AUTO_TEST_CASE(AnAggregateOfFullPlanesIsOneString)
{
        using hash_type = boost::hash2::xxhash_64;
        static_assert(sizeof(plane<64>) == 8UZ);

        auto const p       = make_position<64>();
        auto const message = planes_string<64>(sides * kinds);
        BOOST_CHECK_EQUAL(message.size(), 96UZ);

        // Each side's array is one update, and two updates of the halves are one of the whole.
        auto const h = counted<hash_type>(p);
        BOOST_CHECK_EQUAL(h.updates, 2UZ);
        auto copy = h;
        BOOST_CHECK_EQUAL(copy.result(), string_digest<hash_type>(message));
        auto whole = string_hash<hash_type>(message);
        BOOST_CHECK_EQUAL(xstd::hasher<hash_type>()(p), boost::hash2::get_integral_result<std::size_t>(whole));

        // The same planes nested, built-in or std::array, one contiguous range and so one update.
        plane<64> nested[sides][kinds]; // NOLINT(modernize-avoid-c-arrays): the nesting a user writes
        auto nested_std = std::array<std::array<plane<64>, kinds>, sides>();
        static_assert(sizeof(std::array<plane<64>, kinds>) == kinds * sizeof(plane<64>));
        for (auto const k : std::views::iota(0UZ, kinds)) {
                nested[0][k] = nested_std[0][k] = p.black[k];
                nested[1][k] = nested_std[1][k] = p.white[k];
        }
        for (auto updated : {counted_bits<hash_type>(nested), counted_bits<hash_type>(nested_std)}) {
                BOOST_CHECK_EQUAL(updated.updates, 1UZ);
                BOOST_CHECK_EQUAL(updated.result(), string_digest<hash_type>(message));
        }

        // A flat array, also one update; the two in turn digest as the nested one does.
        auto const flat = counted_bits<hash_type>(p.black);
        BOOST_CHECK_EQUAL(flat.updates, 1UZ);
        auto in_turn = hash_type();
        xstd::bit_hash_append(in_turn, boost::hash2::little_endian_flavor(), nested[0]);
        xstd::bit_hash_append(in_turn, boost::hash2::little_endian_flavor(), nested[1]);
        BOOST_CHECK_EQUAL(in_turn.result(), string_digest<hash_type>(message));
}

// Planes with bytes past ceil(N / 8) go in one at a time, which chunking makes the same digest.
BOOST_AUTO_TEST_CASE(AnAggregateOfPartialPlanesIsTheSameString)
{
        using hash_type = boost::hash2::xxhash_64;
        static_assert(sizeof(plane<50>) == 8UZ);

        auto const p       = make_position<50>();
        auto const message = planes_string<50>(sides * kinds);
        BOOST_CHECK_EQUAL(message.size(), 84UZ);

        auto h = counted<hash_type>(p);
        BOOST_CHECK_EQUAL(h.updates, sides * kinds);
        BOOST_CHECK_EQUAL(h.result(), string_digest<hash_type>(message));

        plane<50> nested[sides][kinds]; // NOLINT(modernize-avoid-c-arrays): the nesting a user writes
        for (auto const k : std::views::iota(0UZ, kinds)) {
                nested[0][k] = p.black[k];
                nested[1][k] = p.white[k];
        }
        auto n = counted_bits<hash_type>(nested);
        BOOST_CHECK_EQUAL(n.updates, sides * kinds);
        BOOST_CHECK_EQUAL(n.result(), string_digest<hash_type>(message));
}

// A range's count follows its strings where its type fixes none, as Hash2 hashes a contiguous range.
BOOST_AUTO_TEST_CASE(ARunTimeCountFollowsThePlanes)
{
        using hash_type = boost::hash2::fnv1a_64;
        auto const p    = make_position<64>();
        auto const half = planes_string<64>(kinds);

        auto const dynamic = counted_bits<hash_type>(std::span<plane<64> const>(p.black));
        BOOST_CHECK_EQUAL(dynamic.updates, 2UZ);
        auto copy = dynamic;
        BOOST_CHECK_EQUAL(copy.result(), string_digest<hash_type>(half, kinds));

        auto const none = counted_bits<hash_type>(std::span<plane<64> const>());
        BOOST_CHECK_EQUAL(none.updates, 1UZ);
        auto none_copy = none;
        BOOST_CHECK_EQUAL(none_copy.result(), string_digest<hash_type>(byte_string(), 0UZ));

        auto const static_span = counted_bits<hash_type>(std::span<plane<64> const, kinds>(p.black));
        BOOST_CHECK_EQUAL(static_span.updates, 1UZ);
        auto static_copy = static_span;
        BOOST_CHECK_EQUAL(static_copy.result(), string_digest<hash_type>(half));

        // Partial planes, a run-time count of them.
        auto const partial = make_position<50>();
        auto spans         = hash_type();
        xstd::bit_hash_append(spans, boost::hash2::little_endian_flavor(), std::span<plane<50> const>(partial.black));
        BOOST_CHECK_EQUAL(spans.result(), string_digest<hash_type>(planes_string<50>(kinds), kinds));

        // No planes in a type that fixes none: Hash2's one byte.
        BOOST_CHECK((record(std::array<plane<64>, 0>()).message == byte_string{0x00}));
        BOOST_CHECK((record(std::array<std::array<plane<64>, 0>, 2>()).message == byte_string{0x00, 0x00}));
}

// A view is no plane's bytes, however wide its object: it goes in as the bits it views.
BOOST_AUTO_TEST_CASE(ARangeOfViewsHashesTheViewedBits)
{
        auto const p     = make_position<64>();
        auto const views = std::array{xstd::bit_set_view(p.black[0]), xstd::bit_set_view(p.black[1])};
        auto const owned = std::array{p.black[0], p.black[1]};
        BOOST_CHECK(record(views).message == record(owned).message);
        BOOST_CHECK(record(views).message == planes_string<64>(2));
}

// A constant expression takes the planes one at a time, to the digest one update gives.
BOOST_AUTO_TEST_CASE(PlanesHashAlikeInAConstantExpression)
{
        constexpr auto full = [] -> boost::hash2::fnv1a_64::result_type {
                auto const p = make_position<64>();
                return bit_digest<boost::hash2::fnv1a_64>(p.black);
        }();
        BOOST_CHECK_EQUAL(bit_digest<boost::hash2::fnv1a_64>(make_position<64>().black), full);
        BOOST_CHECK_EQUAL(full, string_digest<boost::hash2::fnv1a_64>(planes_string<64>(kinds)));
}

// bit_hash_append into H, as xstd::hasher is hash_append into it: seeded as H is, and transparent.
BOOST_AUTO_TEST_CASE(TheHasherIsSeededAndTransparent)
{
        static_assert(std::same_as<xstd::bit_hasher<>::is_transparent, void>);
        static_assert(std::is_nothrow_default_constructible_v<xstd::bit_hasher<boost::hash2::siphash_64>>);

        auto const value      = xstd::bit_fixed_set<100>({1, 3, 99});
        auto const seed_bytes = std::array<unsigned char, 4>{1, 2, 3, 4};
        auto const unseeded   = xstd::bit_hasher<boost::hash2::siphash_64>();
        auto const seeded     = xstd::bit_hasher<boost::hash2::siphash_64>(test_seed);
        auto const keyed      = xstd::bit_hasher<boost::hash2::siphash_64>(seed_bytes.data(), seed_bytes.size());
        BOOST_CHECK(unseeded(value) != seeded(value));
        BOOST_CHECK(seeded(value) != keyed(value));

        auto h = boost::hash2::siphash_64(test_seed);
        xstd::bit_hash_append(h, boost::hash2::default_flavor(), value);
        BOOST_CHECK_EQUAL(seeded(value), boost::hash2::get_integral_result<std::size_t>(h));

        // The bits, not the value as its model has it: no key and no count.
        BOOST_CHECK(unseeded(value) != xstd::hasher<boost::hash2::siphash_64>()(value));

        // Owner and view, owner and owner of other blocks, sequence of either storage: equal bits, equal digests.
        BOOST_CHECK_EQUAL(unseeded(xstd::bit_set_view(value)), unseeded(value));
        BOOST_CHECK_EQUAL(unseeded(xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 100>({1, 3, 99})), unseeded(value));
        BOOST_CHECK_EQUAL(unseeded(alternating(xstd::basic_bit_small_vector<std::uint8_t, 9>(130))), unseeded(alternating(xstd::bit_vector(130))));
        BOOST_CHECK_EQUAL(unseeded(alternating(xstd::basic_bit_bounded_vector<std::uint64_t, 256>(130))), unseeded(alternating(xstd::bit_vector(130))));

        // xxHash by default, as std::hash runs it.
        BOOST_CHECK_EQUAL(xstd::bit_hasher()(value), xstd::bit_hasher<boost::hash2::xxhash_64>()(value));
        BOOST_CHECK_EQUAL(xstd::bit_hasher()(value), std::hash<xstd::bit_fixed_set<100>>()(value));

        // The seeds reach the algorithm, here one that keeps the message and returns its length.
        auto expected = recorder();
        xstd::bit_hash_append(expected, boost::hash2::default_flavor(), value);
        BOOST_CHECK_EQUAL(expected.result(), 13UZ);
        BOOST_CHECK_EQUAL(xstd::bit_hasher<recorder>(test_seed)(value), boost::hash2::get_integral_result<std::size_t>(expected));
        BOOST_CHECK_EQUAL(xstd::bit_hasher<recorder>(seed_bytes.data(), seed_bytes.size())(value), boost::hash2::get_integral_result<std::size_t>(expected));
}

BOOST_AUTO_TEST_SUITE_END()
