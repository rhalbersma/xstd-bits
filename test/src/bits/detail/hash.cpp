//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                  // basic_bit_array, bit_array
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/bit_fixed_set.hpp>              // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_hasher.hpp>                 // bit_hasher
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set, bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/detail/intrin.hpp>              // bools_per_word, byte_bools, expand_word, expand_word_by_table
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <xstd/bits/from_blocks.hpp>                // from_blocks
#include <xstd/misc/ext/boost/hash2.hpp>            // hasher
#include <boost/hash2/flavor.hpp>                   // big_endian_flavor, default_flavor, little_endian_flavor
#include <boost/hash2/fnv1a.hpp>                    // fnv1a_64
#include <boost/hash2/get_integral_result.hpp>      // get_integral_result
#include <boost/hash2/hash_append.hpp>              // hash_append
#include <boost/hash2/siphash.hpp>                  // siphash_64
#include <boost/hash2/xxhash.hpp>                   // xxhash_64
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <boost/unordered/unordered_flat_set.hpp>   // unordered_flat_set
#include <array>                                    // array
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint64_t, uint8_t
#include <functional>                               // hash
#include <initializer_list>                         // initializer_list
#include <ranges>                                   // iota
#include <set>                                      // set
#include <unordered_set>                            // unordered_set
#include <vector>                                   // vector

// The models' messages the hooks append, how they expand bits to bools, and the algorithm std::hash picks.
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

// The packed hasher at an algorithm, which std::hash runs at the one the length picks.
template<class Hash, class T>
[[nodiscard]] auto bit_digest(T const& v)
        -> std::size_t
{
        auto const hasher = xstd::bit_hasher<Hash>();
        return hasher(v);
}

// A seed of the kind a container draws at random, fixed here so that a failure reruns.
constexpr auto test_seed = std::uint64_t{0x9E37'79B9'7F4A'7C15};

// The public hasher, unseeded or seeded, named rather than a temporary.
template<class Hash, class T, class... Seed>
[[nodiscard]] auto xstd_digest(T const& v, Seed... seeds)
        -> std::size_t
{
        auto const hasher = xstd::hasher<Hash>(seeds...);
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
[[nodiscard]] constexpr auto unfolded_digest(T const& v)
        -> Hash::result_type
{
        auto h = Hash();
        boost::hash2::hash_append(h, boost::hash2::default_flavor(), v);
        return h.result();
}

// Every other position set, from the first on, through the subscript every owner of the sequence reading has.
template<class T>
[[nodiscard]] constexpr auto alternating(T x)
        -> T
{
        for (auto i = 0UZ; i < x.size(); i += 2UZ) {
                x[i] = true;
        }
        return x;
}

} // namespace

// std::hash is the packed hasher at xxHash, whatever the length: FNV-1a would leave a last byte's high bits out.
BOOST_AUTO_TEST_CASE(TheDefaultIsTheBitHasherAtXxhash)
{
        auto const fixed   = xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 200>({1, 64, 199});
        auto const growing = dynamic_set({1, 3, 63});
        auto const bools   = alternating(sequence(130));
        BOOST_CHECK_EQUAL(default_digest(fixed), bit_digest<boost::hash2::xxhash_64>(fixed));
        BOOST_CHECK_EQUAL(default_digest(growing), bit_digest<boost::hash2::xxhash_64>(growing));
        BOOST_CHECK_EQUAL(default_digest(bools), bit_digest<boost::hash2::xxhash_64>(bools));
        BOOST_CHECK_EQUAL(default_digest(xstd::bit_set_view(growing)), default_digest(growing));

        // A static width appends no size, so its digest is one on every flavor, folded to std::size_t's width.
        if constexpr (sizeof(std::size_t) == sizeof(std::uint64_t)) {
                BOOST_CHECK_EQUAL(default_digest(fixed), 0xF8C0'7F80'B85C'9EA1ULL);
        }
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

// Every owner and view hashes through xstd::hasher as Hash2 hashes it, under the algorithm and the seed it holds.
BOOST_AUTO_TEST_CASE(XstdHasherRunsTheAlgorithmItIsGiven)
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
        auto const fill = []<class T, class Hasher, template<class...> class Table>(std::initializer_list<T> values) -> void {
                for (auto const& h : {Hasher(), Hasher(test_seed)}) {
                        auto table = Table<T, Hasher>(0, h);
                        for (auto const& v : values) {
                                table.insert(v);
                        }
                        BOOST_CHECK_EQUAL(table.size(), 2UZ);
                        for (auto const& v : values) {
                                BOOST_CHECK(table.contains(v));
                        }
                }
        };
        auto const fill_all = [&]<class T>(std::initializer_list<T> values) -> void {
                fill.template operator()<T, xstd::hasher<boost::hash2::siphash_64>, std::unordered_set>(values);
                fill.template operator()<T, xstd::hasher<boost::hash2::siphash_64>, boost::unordered_flat_set>(values);
                fill.template operator()<T, xstd::bit_hasher<boost::hash2::siphash_64>, std::unordered_set>(values);
                fill.template operator()<T, xstd::bit_hasher<boost::hash2::siphash_64>, boost::unordered_flat_set>(values);
        };

        // Three values, two of them equal, so each table holds two.
        fill_all.operator()<xstd::bit_fixed_set<100>>({xstd::bit_fixed_set<100>({1, 99}), xstd::bit_fixed_set<100>({1, 99}), xstd::bit_fixed_set<100>({2})});
        fill_all.operator()<xstd::bit_set>({xstd::bit_set({1, 200}), xstd::bit_set({1, 200}), xstd::bit_set({2})});
        fill_all.operator()<xstd::bit_vector>({alternating(xstd::bit_vector(130)), alternating(xstd::bit_vector(130)), xstd::bit_vector(130)});

        // A view keys the table as its owner's value, so a second view of equal bits is found, not added.
        auto const first  = xstd::bit_set({1, 200});
        auto const second = xstd::bit_set({1, 200});
        auto const third  = xstd::bit_set({2});
        using view_type   = decltype(xstd::bit_set_view(first));
        fill_all.operator()<view_type>({xstd::bit_set_view(first), xstd::bit_set_view(second), xstd::bit_set_view(third)});
}

// The models' bytes: the bools of a sequence, no size at a static width, and a set's keys under the flavor's order.
BOOST_AUTO_TEST_CASE(TheHooksAppendTheModelsMessage)
{
        BOOST_CHECK((record<boost::hash2::little_endian_flavor>(alternating(sequence(6))).bytes == std::vector<unsigned char>{1, 0, 1, 0, 1, 0, 6, 0, 0, 0}));
        BOOST_CHECK((record<boost::hash2::little_endian_flavor>(alternating(xstd::bit_array<6>())).bytes == std::vector<unsigned char>{1, 0, 1, 0, 1, 0}));
        BOOST_CHECK((record<boost::hash2::little_endian_flavor>(xstd::bit_array<0>()).bytes == std::vector<unsigned char>{0}));

        auto const keys = std::set<std::size_t>({1, 3, 5});
        BOOST_CHECK(record<boost::hash2::little_endian_flavor>(fixed_set({1, 3, 5})).bytes == record<boost::hash2::little_endian_flavor>(keys).bytes);
        BOOST_CHECK(record<boost::hash2::big_endian_flavor>(dynamic_set({1, 3, 5})).bytes == record<boost::hash2::big_endian_flavor>(keys).bytes);
}

// Bools go in eight words to an update, the last one stopping at the width.
BOOST_AUTO_TEST_CASE(TheBoolsGoInEightWordsToAnUpdate)
{
        auto const value = alternating(sequence(1'000));
        auto const h     = record<boost::hash2::little_endian_flavor>(value);
        BOOST_CHECK_EQUAL(h.updates, 3UZ);
        BOOST_CHECK_EQUAL(h.bytes.size(), 1'004UZ);
        BOOST_CHECK(record<boost::hash2::little_endian_flavor>(value).bytes == record<boost::hash2::little_endian_flavor>(std::vector<bool>(value.begin(), value.end())).bytes);
}

// The table, and whatever instruction the target has, write the same bools.
BOOST_AUTO_TEST_CASE(EveryExpansionAgrees)
{
        for (auto const byte : std::views::iota(0UZ, xstd::bits::detail::byte_bools.size())) {
                for (auto const i : std::views::iota(0UZ, xstd::bits::detail::byte_bools[byte].size())) {
                        BOOST_CHECK_EQUAL(xstd::bits::detail::byte_bools[byte][i], (byte >> i) & 1UZ);
                }
        }
        for (auto const word : {std::uint64_t{0}, ~std::uint64_t{0}, std::uint64_t{0x0123'4567'89AB'CDEF}, std::uint64_t{0x8000'0000'0000'0001}}) {
                auto by_table       = std::array<unsigned char, xstd::bits::detail::bools_per_word>();
                auto by_instruction = std::array<unsigned char, xstd::bits::detail::bools_per_word>();
                xstd::bits::detail::expand_word_by_table(word, by_table);
                xstd::bits::detail::expand_word(word, by_instruction);
                BOOST_CHECK(by_table == by_instruction);
        }
}

// A constant expression expands by the table, to the digest the model has.
BOOST_AUTO_TEST_CASE(TheHooksAgreeInAConstantExpression)
{
        static_assert(unfolded_digest<boost::hash2::fnv1a_64>(alternating(sequence(130))) == unfolded_digest<boost::hash2::fnv1a_64>(alternating(std::vector<bool>(130))));
        static_assert(unfolded_digest<boost::hash2::fnv1a_64>(alternating(xstd::basic_bit_array<std::uint8_t, 70>())) == unfolded_digest<boost::hash2::fnv1a_64>(alternating(std::array<bool, 70>())));
        BOOST_CHECK_EQUAL(unfolded_digest<boost::hash2::fnv1a_64>(alternating(sequence(130))), unfolded_digest<boost::hash2::fnv1a_64>(alternating(std::vector<bool>(130))));
}

// Equal values hash equal under the default whatever holds them, at a short string and at a long one.
BOOST_AUTO_TEST_CASE(EqualValuesHashEqualUnderTheDefaultAcrossStorages)
{
        // The set reading at a static width: an owner, a view over it, and a view over borrowed blocks of another type.
        auto const fixed_set_digests = []<std::size_t N, std::size_t K>(std::initializer_list<std::size_t> keys, std::array<std::uint8_t, K> const& blocks) -> std::array<std::size_t, 3> {
                auto const owned = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, N>(keys);
                auto const view  = xstd::bit_set_view(owned);
                auto const lent  = xstd::bit_set_view(blocks);
                return {default_digest(owned), default_digest(view), default_digest(lent)};
        };
        auto const short_fixed = fixed_set_digests.operator()<64>({1, 3, 63}, std::array<std::uint8_t, 8>{0x0A, 0, 0, 0, 0, 0, 0, 0x80});
        auto const long_fixed  = fixed_set_digests.operator()<128>({1, 3, 127}, std::array<std::uint8_t, 16>{0x0A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80});
        BOOST_CHECK(short_fixed[0] == short_fixed[1] and short_fixed[1] == short_fixed[2]);
        BOOST_CHECK(long_fixed[0] == long_fixed[1] and long_fixed[1] == long_fixed[2]);

        // The set reading at a run-time width: every resizable owner, and a view over one.
        auto const dynamic_set_digests = [](std::initializer_list<std::size_t> keys) -> std::array<std::size_t, 4> {
                auto const growing      = xstd::basic_bit_set<std::size_t, std::uint8_t>(keys);
                auto const bounded      = xstd::basic_bit_bounded_set<std::size_t, std::uint64_t, 256>(keys);
                auto const inline_first = xstd::basic_bit_small_set<std::size_t, std::uint64_t, 256>(keys);
                auto const view         = xstd::bit_set_view(growing);
                return {default_digest(growing), default_digest(bounded), default_digest(inline_first), default_digest(view)};
        };
        for (auto const& digests : {dynamic_set_digests({1, 3}), dynamic_set_digests({1, 3, 5, 200})}) {
                BOOST_CHECK(digests[0] == digests[1] and digests[1] == digests[2] and digests[2] == digests[3]);
        }

        // The sequence reading at a run-time width, every owner; a static one is std::array's and has no size.
        auto const sequence_digests = []<std::size_t N> -> std::array<std::size_t, 3> {
                auto const growing      = alternating(xstd::basic_bit_vector<std::uint8_t>(N));
                auto const bounded      = alternating(xstd::basic_bit_bounded_vector<std::uint64_t, 256>(N));
                auto const inline_first = alternating(xstd::basic_bit_small_vector<std::uint64_t, 256>(N));
                return {default_digest(growing), default_digest(bounded), default_digest(inline_first)};
        };
        for (auto const& digests : {sequence_digests.operator()<10>(), sequence_digests.operator()<200>()}) {
                BOOST_CHECK(digests[0] == digests[1] and digests[1] == digests[2]);
        }
        BOOST_CHECK_EQUAL(default_digest(alternating(xstd::basic_bit_array<std::uint8_t, 200>())), default_digest(alternating(xstd::basic_bit_array<std::uint64_t, 200>())));
}

BOOST_AUTO_TEST_SUITE_END()
