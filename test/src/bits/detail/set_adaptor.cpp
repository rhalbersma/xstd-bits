//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bit_exchange.hpp>                       // converts_from, converts_to
#include <test/for_each_type.hpp>                      // for_each_type
#include <test/set/enums.hpp>                          // day, level, piece, sign
#include <test/set/lookup.hpp>                         // lookup_mismatches
#include <test/set/primitives.hpp>                     // heterogeneous_key
#include <test/spec/random.hpp>                        // engine, seed
#include <xstd/bits/bit/bit_convert.hpp>               // bit_convert
#include <xstd/bits/bit_bounded_set.hpp>               // basic_bit_bounded_set
#include <xstd/bits/bit_concepts/bit_mask_mapping.hpp> // bit_mask_mapping
#include <xstd/bits/bit_fixed_set.hpp>                 // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp>              // bit_flag_mapping
#include <xstd/bits/bit_key_mapping.hpp>               // bit_key_mapping, enum_traits
#include <xstd/bits/bit_set.hpp>                       // basic_bit_set, bit_set
#include <xstd/bits/bit_set_view.hpp>                  // bit_set_view
#include <xstd/bits/detail/bit_block_container.hpp>    // bit_block_container
#include <xstd/bits/detail/ownership.hpp>              // owned_bits_t, storage
#include <xstd/bits/detail/set_adaptor.hpp>            // set_adaptor
#include <xstd/bits/ext/boost/bit_small_set.hpp>       // basic_bit_small_set
#include <boost/test/unit_test.hpp>                    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_TEST_CONTEXT
#include <algorithm>                                   // lexicographical_compare_three_way, max, min, ranges::all_of, ranges::equal, ranges::includes
#include <array>                                       // array
#include <bitset>                                      // bitset
#include <compare>                                     // strong_ordering
#include <concepts>                                    // copyable, equality_comparable, invocable, regular, same_as, totally_ordered
#include <cstddef>                                     // ptrdiff_t, size_t
#include <cstdint>                                     // uint8_t, uint32_t, uint64_t
#include <functional>                                  // greater, greater_equal, less, less_equal, ranges::less
#include <initializer_list>                            // initializer_list
#include <iterator>                                    // ranges::distance
#include <limits>                                      // numeric_limits
#include <ranges>                                      // bidirectional_range, from_range, iota, ranges::to, transform
#include <set>                                         // set
#include <stdexcept>                                   // length_error, out_of_range
#include <tuple>                                       // tuple, tuple_cat
#include <type_traits>                                 // underlying_type_t
#include <utility>                                     // declval, to_underlying
#include <vector>                                      // vector

namespace {

using Storage = xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 2>, 100>;
using Owner   = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 100>;
using View    = xstd::bits::detail::set_adaptor<Storage, xstd::bits::detail::storage::borrowed>;
using Reader  = xstd::bits::detail::set_adaptor<Storage const, xstd::bits::detail::storage::borrowed>;

// Dependent, so an absent member is a false rather than a hard error.
template<class S>
constexpr bool can_insert = requires (S s) { s.insert(0UZ); };

template<class S>
constexpr bool can_erase = requires (S s) { s.erase(0UZ); };

template<class S>
constexpr bool can_clear = requires (S s) { s.clear(); };

template<class S>
constexpr bool can_fill = requires (S s) { s.fill(); s.complement(); s.complement(0UZ); };

template<class S>
constexpr bool can_swap = requires (S s) { s.swap(s); };

template<class S>
constexpr bool has_complement = requires (S s) { ~s; s & s; };

// What for_each accepts, dependent so a rejected functor is a false rather than a hard error.
template<class S, class F>
constexpr bool walks = requires (S const& s, F f) { s.for_each(f); };

template<class S, class F>
constexpr bool walks_reverse = requires (S const& s, F f) { s.for_each_reverse(f); };

// Functors overloaded on the value category.
struct void_probe
{
        bool& took_a_reference;

        auto operator()(std::size_t&&) const -> void {}

        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(std::size_t&) const
                -> void
        {
                took_a_reference = true;
        }
};

struct bool_probe
{
        bool& took_a_reference;

        auto operator()(std::size_t&&) const
                -> bool
        {
                return true;
        }

        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(std::size_t&) const
                -> bool
        {
                took_a_reference = true;
                return true;
        }
};

template<class Set>
[[nodiscard]] auto keys(Set const& s)
        -> std::set<std::size_t>
{
        return {s.begin(), s.end()};
}

// Every reading-level question a view can answer, against std::set answering the same one.
template<class Set>
auto check_whole(Set const& s, std::set<std::size_t> const& model)
        -> void
{
        BOOST_CHECK(keys(s) == model);
        BOOST_CHECK_EQUAL(s.size(), model.size());
        BOOST_CHECK_EQUAL(s.empty(), model.empty());
        if (not model.empty()) {
                BOOST_CHECK_EQUAL(static_cast<std::size_t>(s.front()), *model.begin());
                BOOST_CHECK_EQUAL(static_cast<std::size_t>(s.back()), *model.rbegin());
        }
}

template<class Set>
auto check_key(Set const& s, std::set<std::size_t> const& model, std::size_t x)
        -> void
{
        BOOST_CHECK_EQUAL(s.contains(x), model.contains(x));
        BOOST_CHECK_EQUAL(s.count(x), model.count(x));
        BOOST_CHECK((s.find(x) == s.end()) == not model.contains(x));

        auto const lower = model.lower_bound(x);
        BOOST_CHECK((s.lower_bound(x) == s.end()) == (lower == model.end()));
        if (lower != model.end()) {
                BOOST_CHECK_EQUAL(static_cast<std::size_t>(*s.lower_bound(x)), *lower);
        }

        auto const upper = model.upper_bound(x);
        BOOST_CHECK((s.upper_bound(x) == s.end()) == (upper == model.end()));
        if (upper != model.end()) {
                BOOST_CHECK_EQUAL(static_cast<std::size_t>(*s.upper_bound(x)), *upper);
        }

        auto const [first, last] = s.equal_range(x);
        BOOST_CHECK(first == s.lower_bound(x) and last == s.upper_bound(x));
}

template<class Set>
auto check_reads(Set const& s, std::set<std::size_t> const& model, std::size_t width)
        -> void
{
        check_whole(s, model);
        for (auto const x : std::views::iota(0UZ, width + 2UZ)) {
                check_key(s, model, x);
        }
}

} // namespace

BOOST_AUTO_TEST_SUITE(SetAdaptor)

// Dependent, so an operator that is missing makes this false rather than ill-formed.
template<class X, class Y>
constexpr bool compares_with = requires (X const& x, Y const& y) { x == y; };

// Each owner wraps the storage its base clause names, and two names over one storage are two types that never compare.
BOOST_AUTO_TEST_CASE(AnOwnerIsATypeOfItsOwnOverItsStorage)
{
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 20>>, xstd::bits::detail::bit_block_container<std::array<std::uint8_t, 3>, 20>>);
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bit_set<std::size_t, std::uint32_t>>, xstd::bits::detail::bit_block_container<std::vector<std::uint32_t>>>);
        static_assert(not compares_with<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 24>, xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<std::array<std::uint8_t, 3>>>>);
        static_assert(not compares_with<xstd::basic_bit_set<std::size_t, std::uint32_t>, xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<std::vector<std::uint32_t>>>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(AnOwnerIsRegularAndAViewIsCopyable)
{
        static_assert(std::regular<Owner>);
        static_assert(std::totally_ordered<Owner>);
        static_assert(std::ranges::bidirectional_range<Owner>);

        static_assert(std::copyable<View> and std::copyable<Reader>);
        static_assert(std::totally_ordered<View>);
        static_assert(std::ranges::bidirectional_range<View> and std::ranges::bidirectional_range<Reader>);
        static_assert(not std::default_initializable<View>);
}

// Where the trait does not let this handle write, the member is not there to call.
BOOST_AUTO_TEST_CASE(WritingIsGatedByTheStorageNotByThisConst)
{
        static_assert(can_insert<View> and can_erase<View> and can_clear<View> and can_fill<View>);
        static_assert(can_insert<View const> and can_erase<View const> and can_clear<View const>);
        static_assert(can_insert<Owner> and can_swap<Owner> and has_complement<Owner>);

        static_assert(not can_insert<Reader> and not can_erase<Reader> and not can_clear<Reader> and not can_fill<Reader>);
        static_assert(not can_insert<Owner const> and not can_erase<Owner const> and not can_clear<Owner const>);
        static_assert(not can_swap<View> and not has_complement<View>);
}

BOOST_AUTO_TEST_CASE(AViewWritesThroughToWhatItViews)
{
        auto c       = Storage();
        auto const v = View(c);

        auto const [it, inserted] = v.insert(3UZ);
        BOOST_CHECK(inserted and *it == 3UZ and c.test(3));
        BOOST_CHECK(not v.insert(3UZ).second);
        BOOST_CHECK(v.emplace(5UZ).second and c.test(5));
        BOOST_CHECK(*v.emplace_hint(v.end(), 7UZ) == 7UZ and c.test(7));
        BOOST_CHECK(*v.insert(v.end(), 9UZ) == 9UZ);
        v.insert({11UZ, 13UZ});
        auto const some = std::vector<std::size_t>{17UZ, 19UZ};
        v.insert_range(some);
        auto const nothing = std::vector<std::size_t>();
        v.insert(nothing.begin(), nothing.end());
        BOOST_CHECK(keys(v) == std::set<std::size_t>({3, 5, 7, 9, 11, 13, 17, 19}));

        BOOST_CHECK_EQUAL(v.erase(5UZ), 1UZ);
        BOOST_CHECK_EQUAL(v.erase(5UZ), 0UZ);
        BOOST_CHECK(*v.erase(v.find(7UZ)) == 9UZ);
        BOOST_CHECK(v.erase(v.find(11UZ), v.find(19UZ)) == v.find(19UZ));
        BOOST_CHECK(keys(v) == std::set<std::size_t>({3, 9, 19}));

        v.complement(19UZ);
        v.complement(20UZ);
        BOOST_CHECK(keys(v) == std::set<std::size_t>({3, 9, 20}));

        v.clear();
        BOOST_CHECK(c.none());
        v.fill();
        BOOST_CHECK(c.all());
        v.complement();
        BOOST_CHECK(c.none());
}

// A view is shallow: a const view writes, a view over const storage does not, and a copy of a view is the same view.
BOOST_AUTO_TEST_CASE(AViewIsShallow)
{
        auto c = Storage();
        View const v(c);
        v.insert(42UZ);
        auto const w = v;
        BOOST_CHECK(w.contains(42UZ) and v == w);

        Reader const r(c);
        BOOST_CHECK(r.contains(42UZ) and keys(r) == keys(v));
        BOOST_CHECK(r.is_subset_of(r) and not r.is_proper_subset_of(r) and intersects(r, r));
        BOOST_CHECK(r.is_superset_of(r) and not r.is_proper_superset_of(r));
}

BOOST_AUTO_TEST_CASE(TheViewsAnswerEveryReadOverEveryStorage)
{
        for (auto const& model : {std::set<std::size_t>{}, {0UZ}, {3UZ, 63UZ, 64UZ, 99UZ}, {99UZ}}) {
                auto a = Storage();
                auto v = xstd::bits::detail::bit_block_container<std::vector<std::uint64_t>>(100UZ);
                for (auto const p : model) {
                        a.set(p);
                        v.set(p);
                }
                check_reads(View(a), model, 100UZ);
                check_reads(xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<std::vector<std::uint64_t>>, xstd::bits::detail::storage::borrowed>(v), model, 100UZ);
        }
}

// max_size is the positions there are to hold: the width, what an owner can address, or what a view looks at.
BOOST_AUTO_TEST_CASE(MaxSizeIsThePositionsThereAreToHold)
{
        auto storage = Storage();
        static_assert(Owner::max_size() == 100UZ);
        BOOST_CHECK_EQUAL(View(storage).max_size(), 100UZ);

        // An owner grows to what its storage can address, which is whole blocks of it and never the address space.
        using Heap = xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<std::vector<std::uint64_t>>, xstd::bits::detail::storage::owned>;
        BOOST_CHECK_EQUAL(Heap().max_size(), xstd::bits::detail::bit_block_container<std::vector<std::uint64_t>>().max_size());
        BOOST_CHECK_LT(Heap().max_size(), std::numeric_limits<std::size_t>::max());

        // A view cannot grow what it views, so its max_size is that width -- and filling it is what full() means.
        auto v          = xstd::bits::detail::bit_block_container<std::vector<std::uint64_t>>(10UZ);
        auto const view = xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<std::vector<std::uint64_t>>, xstd::bits::detail::storage::borrowed>(v);
        BOOST_CHECK_EQUAL(view.max_size(), 10UZ);
        BOOST_CHECK(not view.full());
        view.fill();
        BOOST_CHECK(view.full());
        BOOST_CHECK_EQUAL(view.size(), 10UZ);

        // A run-time width, read through the view over it.
        using Dynamic = xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<std::vector<std::uint64_t>>, xstd::bits::detail::storage::borrowed>;
        auto b        = xstd::bits::detail::bit_block_container<std::vector<std::uint64_t>>(9UZ);
        BOOST_CHECK_EQUAL(Dynamic(b).max_size(), 9UZ);
}

// The set operations use the storage's members where it has them, and its bulk operators where it has not.
BOOST_AUTO_TEST_CASE(TheSetPredicatesAgreeAcrossStorages)
{
        using Small = xstd::bits::detail::bit_block_container<std::array<std::uint64_t, 1>, 9>;
        auto a      = Small();
        auto b      = Small();
        auto e      = Small();
        a.set(1);
        b.set(1);
        b.set(3);
        using S      = xstd::bits::detail::set_adaptor<Small, xstd::bits::detail::storage::borrowed>;
        auto const x = S(a);
        auto const y = S(b);

        BOOST_CHECK(x.is_subset_of(y) and not y.is_subset_of(x));
        BOOST_CHECK(x.is_proper_subset_of(y) and not x.is_proper_subset_of(x) and not y.is_proper_subset_of(x));
        BOOST_CHECK(y.is_superset_of(x) and not x.is_superset_of(y));
        BOOST_CHECK(y.is_proper_superset_of(x) and not x.is_proper_superset_of(x) and not x.is_proper_superset_of(y));
        BOOST_CHECK(intersects(x, y) and not intersects(x, S(e)));
        BOOST_CHECK(x != y and x < y);
        BOOST_CHECK((x <=> y) == std::strong_ordering::less);

        x.insert(3UZ);
        BOOST_CHECK(x == y and not x.is_proper_subset_of(y) and not y.is_proper_superset_of(x));
}

// The ordering invariant: the block-wise entry and the iterators agree, and the fallback is the invariant itself.
BOOST_AUTO_TEST_CASE(TheOrderingIsTheLexicographicOrderOfTheKeys)
{
        auto const patterns = std::vector<std::set<std::size_t>>{{}, {0}, {1}, {0, 1}, {0, 1, 99}, {63, 64}, {64}, {99}};
        for (auto const& p : patterns) {
                for (auto const& q : patterns) {
                        auto const x = Owner(p.begin(), p.end());
                        auto const y = Owner(q.begin(), q.end());
                        BOOST_CHECK((x <=> y) == std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()));
                        BOOST_CHECK((x <=> y) == (p <=> q));

                        auto s = Storage();
                        auto t = Storage();
                        for (auto const i : p) {
                                s.set(i);
                        }
                        for (auto const i : q) {
                                t.set(i);
                        }
                        BOOST_CHECK((View(s) <=> View(t)) == (p <=> q));
                        BOOST_CHECK((View(s) == View(t)) == (p == q));
                }
        }
}

BOOST_AUTO_TEST_CASE(TheNonMemberFormsAreTheOwners)
{
        auto const x = Owner{1, 2, 3};
        auto const y = Owner{2, 3, 4};
        BOOST_CHECK(keys(x & y) == std::set<std::size_t>({2, 3}));
        BOOST_CHECK(keys(x | y) == std::set<std::size_t>({1, 2, 3, 4}));
        BOOST_CHECK(keys(x ^ y) == std::set<std::size_t>({1, 4}));
        BOOST_CHECK(keys(x - y) == std::set<std::size_t>({1}));
        BOOST_CHECK(keys(x << 1) == std::set<std::size_t>({2, 3, 4}));
        BOOST_CHECK(keys(x >> 1) == std::set<std::size_t>({0, 1, 2}));
        BOOST_CHECK((~x).size() == 97UZ and not(~x).contains(2UZ));

        auto z = x;
        z      = {5, 6};
        BOOST_CHECK(keys(z) == std::set<std::size_t>({5, 6}));
        BOOST_CHECK_EQUAL(erase_if(z, [](auto k) { return k == 5UZ; }), 1UZ);
        BOOST_CHECK(keys(z) == std::set<std::size_t>({6}));
        swap(z, z);
        BOOST_CHECK(keys(z) == std::set<std::size_t>({6}));
}

// insert_range takes a tier above the element-wise loop where it can; every case here answers the element-wise one.
BOOST_AUTO_TEST_CASE(RangedInsertionAgreesWithTheElementwiseLoop)
{
        constexpr auto N = 100UZ;

        // The consecutive tier, over every [lo, hi) the width admits.
        for (auto const lo : std::views::iota(0UZ, N + 1UZ)) {
                for (auto const hi : std::views::iota(lo, N + 1UZ)) {
                        auto ranged = xstd::bit_fixed_set<N>();
                        ranged.insert_range(std::views::iota(lo, hi));

                        auto elementwise = xstd::bit_fixed_set<N>();
                        for (auto const i : std::views::iota(lo, hi)) {
                                elementwise.insert(i);
                        }
                        BOOST_CHECK(ranged == elementwise);
                }
        }

        // It has to leave what lies outside the range alone, which a whole-block write would not.
        auto seeded = xstd::bit_fixed_set<N>();
        seeded.insert(0UZ);
        seeded.insert(70UZ);
        seeded.insert(99UZ);
        auto expected = seeded;
        seeded.insert_range(std::views::iota(10UZ, 65UZ));
        for (auto const i : std::views::iota(10UZ, 65UZ)) {
                expected.insert(i);
        }
        BOOST_CHECK(seeded == expected);

        // The set tier: a union, and equally the element-wise answer.
        auto lhs = xstd::bit_fixed_set<N>();
        lhs.insert(1UZ);
        lhs.insert(64UZ);
        auto rhs = xstd::bit_fixed_set<N>();
        rhs.insert(64UZ);
        rhs.insert(99UZ);
        auto united = lhs;
        united.insert_range(rhs);
        for (auto const x : rhs) {
                lhs.insert(x);
        }
        BOOST_CHECK(united == lhs);
}

// The dynamic width grows to hold what is inserted, and the ranged tier must grow the same way the loop does.
BOOST_AUTO_TEST_CASE(RangedInsertionGrowsADynamicWidth)
{
        auto ranged = xstd::bit_set();
        ranged.insert_range(std::views::iota(5UZ, 130UZ));

        auto elementwise = xstd::bit_set();
        for (auto const i : std::views::iota(5UZ, 130UZ)) {
                elementwise.insert(i);
        }
        BOOST_CHECK(ranged == elementwise);
        BOOST_CHECK_EQUAL(ranged.size(), 125UZ);

        // Appending a second, disjoint stretch grows it again and keeps the first.
        ranged.insert_range(std::views::iota(200UZ, 260UZ));
        for (auto const i : std::views::iota(200UZ, 260UZ)) {
                elementwise.insert(i);
        }
        BOOST_CHECK(ranged == elementwise);
}

namespace {

// One refusing write as a function: six BOOST_CHECK_THROW bodies would pass readability-function-cognitive-complexity.
auto check_refuses(std::invocable auto write)
        -> void
{
        BOOST_CHECK_THROW(write(), std::out_of_range);
}

} // namespace

// The one key a set can be unable to hold, every other member being total over key_type.
BOOST_AUTO_TEST_CASE(AKeyAStaticWidthCannotHoldIsOutOfRange)
{
        using S = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 100>;
        static_assert(S::max_size() == 100UZ);

        auto s = S();
        s.insert(3UZ);

        check_refuses([&] -> void { static_cast<void>(s.insert(100UZ)); });
        check_refuses([&] -> void { static_cast<void>(s.insert(500UZ)); });
        check_refuses([&] -> void { static_cast<void>(s.emplace(500UZ)); });
        check_refuses([&] -> void { s.emplace_hint(s.begin(), 500UZ); });
        check_refuses([&] -> void { s.insert(s.begin(), 500UZ); });
        check_refuses([&] -> void { s.complement(500UZ); });

        // A refused key writes nothing, and the last one it can hold is 99, which both writes take.
        BOOST_CHECK_EQUAL(s.size(), 1UZ);
        s.insert(99UZ);
        s.complement(98UZ);
        BOOST_CHECK(s.contains(3UZ) and s.contains(99UZ) and s.contains(98UZ));
        BOOST_CHECK_EQUAL(s.size(), 3UZ);
}

// The bulk inserts refuse it too, the consecutive tier guarding the range's last position before it writes anything.
BOOST_AUTO_TEST_CASE(TheBulkInsertsRefuseTheKeyAndSayWhatTheyWrote)
{
        using S = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 100>;

        auto consecutive = S();
        BOOST_CHECK_THROW(consecutive.insert_range(std::views::iota(98UZ, 102UZ)), std::out_of_range);
        BOOST_CHECK(consecutive.empty());

        auto elementwise = S();
        BOOST_CHECK_THROW(elementwise.insert({1UZ, 500UZ}), std::out_of_range);
        BOOST_CHECK_EQUAL(elementwise.size(), 1UZ);
        BOOST_CHECK(elementwise.contains(1UZ));
}

// Asking stays total, which is what [set] gives it: a key past the width is one the set does not hold.
BOOST_AUTO_TEST_CASE(AKeyPastTheWidthIsStillAskable)
{
        using S = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 100>;

        auto s = S();
        s.insert(3UZ);

        BOOST_CHECK(not s.contains(500UZ));
        BOOST_CHECK_EQUAL(s.count(500UZ), 0UZ);
        BOOST_CHECK(s.find(500UZ) == s.end()); // NOLINT(readability-container-contains): find's totality is the check, which contains cannot show
        BOOST_CHECK(s.lower_bound(500UZ) == s.end());
        BOOST_CHECK(s.upper_bound(500UZ) == s.end());
        BOOST_CHECK_EQUAL(s.erase(500UZ), 0UZ);
        BOOST_CHECK_EQUAL(s.size(), 1UZ);
}

// The same key on the other two storages: a dynamic extent grows to admit it, and complement grows where insert grows.
BOOST_AUTO_TEST_CASE(ADynamicWidthAdmitsTheKeyInstead)
{
        auto d = xstd::bit_set();
        d.insert(500UZ);
        BOOST_CHECK(d.contains(500UZ));

        d.complement(1000UZ);
        BOOST_CHECK(d.contains(1000UZ));
        BOOST_CHECK_EQUAL(d.size(), 2UZ);

        // And toggling it back is the erase, without shrinking.
        d.complement(1000UZ);
        BOOST_CHECK(not d.contains(1000UZ));
        BOOST_CHECK_EQUAL(d.size(), 1UZ);
}

namespace {

// One-bit values keying a set by their bit, six of the eight bits of their block.
enum class flag : std::uint8_t
{
        a = 0x01,
        b = 0x02,
        c = 0x04,
        d = 0x08,
        e = 0x10,
        f = 0x20,
};

// Every owning column over one closed mapping, as wide as its keys: a width, the heap, a capacity and inline blocks.
template<class Key, std::size_t N, class KeyMapping = xstd::bit_key_mapping<Key>>
using closed_columns = std::tuple<xstd::basic_bit_fixed_set<Key, std::uint8_t, N, KeyMapping>, xstd::basic_bit_set<Key, std::uint8_t, KeyMapping>, xstd::basic_bit_bounded_set<Key, std::uint8_t, N, KeyMapping>, xstd::basic_bit_small_set<Key, std::uint8_t, N, KeyMapping>, xstd::basic_bit_set<Key, std::uint8_t, KeyMapping, std::greater<>>>;

// A range from zero with padding, one from a negative value, a list with gaps, a list across zero, and one-bit values.
using closed_owners = decltype(std::tuple_cat(std::declval<closed_columns<test::set::day, 5UZ>>(), std::declval<closed_columns<test::set::level, 3UZ>>(), std::declval<closed_columns<test::set::piece, 6UZ>>(), std::declval<closed_columns<test::set::sign, 3UZ>>(), std::declval<closed_columns<flag, 6UZ, xstd::bit_flag_mapping<flag, 6UZ>>>()));

// Every key of the universe, in position order.
template<class X>
auto universe()
        -> std::vector<typename X::key_type>
{
        using key_type = X::key_type;
        using mapping  = X::key_mapping_type;
        return std::views::iota(0UZ, mapping::size) | std::views::transform([](std::size_t i) -> key_type { return mapping::from_index(i); }) | std::ranges::to<std::vector>();
}

// The values a write is offered: every one-bit value of a flag, else each value from two below the keys to two above.
template<class Key>
auto offered()
        -> std::vector<Key>
{
        if constexpr (std::same_as<Key, flag>) {
                return std::array<std::uint8_t, 8>{0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80} | std::views::transform([](std::uint8_t v) -> flag { return static_cast<flag>(v); }) | std::ranges::to<std::vector>();
        } else {
                using underlying      = std::underlying_type_t<Key>;
                constexpr auto values = xstd::enum_traits<Key>::values;
                auto const lo         = std::max(int{std::to_underlying(values.front())} - 2, int{std::numeric_limits<underlying>::min()});
                auto const hi         = std::min(int{std::to_underlying(values.back())} + 2, int{std::numeric_limits<underlying>::max()});
                return std::views::iota(lo, hi + 1) | std::views::transform([](int v) -> Key { return static_cast<Key>(v); }) | std::ranges::to<std::vector>();
        }
}

// Each way a value enters a set, offered one that is no key: each refuses it as out_of_range and writes nothing.
template<class X>
auto refuses_at_every_door(X const& a, typename X::key_type v)
        -> void
{
        auto x         = a;
        auto const one = std::array{v};
        check_refuses([&] -> void { static_cast<void>(x.insert(v)); });
        check_refuses([&] -> void { x.insert(x.begin(), v); });
        check_refuses([&] -> void { static_cast<void>(x.emplace(v)); });
        check_refuses([&] -> void { x.emplace_hint(x.begin(), v); });
        check_refuses([&] -> void { x.insert({v}); });
        check_refuses([&] -> void { x.insert(one.begin(), one.end()); });
        check_refuses([&] -> void { x.insert_range(one); });
        check_refuses([&] -> void { x.complement(v); });
        check_refuses([&] -> void { static_cast<void>(X(one.begin(), one.end())); });
        check_refuses([&] -> void { static_cast<void>(X(std::from_range, one)); });
        BOOST_CHECK(x == a);
}

// The keys a set holds, as its model holds them.
template<class Model, class X>
[[nodiscard]] auto modelled(X const& x)
        -> Model
{
        return Model(x.begin(), x.end());
}

// Each way a value enters a set, offered a key: each writes it as the model's insert does, and complement toggles it.
template<class X, class Model>
auto admits_at_every_door(X const& a, Model const& model, typename X::key_type v)
        -> void
{
        auto with = model;
        with.insert(v);
        auto without = model;
        without.erase(v);
        auto const one       = std::array{v};
        auto const writes_it = [&](auto write) -> bool {
                auto x = a;
                write(x);
                return modelled<Model>(x) == with;
        };
        BOOST_CHECK(writes_it([&](X& x) -> void { static_cast<void>(x.insert(v)); }));
        BOOST_CHECK(writes_it([&](X& x) -> void { x.insert(x.begin(), v); }));
        BOOST_CHECK(writes_it([&](X& x) -> void { static_cast<void>(x.emplace(v)); }));
        BOOST_CHECK(writes_it([&](X& x) -> void { x.emplace_hint(x.begin(), v); }));
        BOOST_CHECK(writes_it([&](X& x) -> void { x.insert({v}); }));
        BOOST_CHECK(writes_it([&](X& x) -> void { x.insert(one.begin(), one.end()); }));
        BOOST_CHECK(writes_it([&](X& x) -> void { x.insert_range(one); }));
        auto x = a;
        x.complement(v);
        BOOST_CHECK(modelled<Model>(x) == (model.contains(v) ? without : with));
        auto const from_iterators = X(one.begin(), one.end());
        auto const from_range     = X(std::from_range, one);
        BOOST_CHECK(modelled<Model>(from_iterators) == Model{v});
        BOOST_CHECK(modelled<Model>(from_range) == Model{v});
}

// An enumerator is a one-element set: the operators that write it refuse it, and the two that only read it answer.
template<class X>
auto meets_it_as_an_empty_set_or_refuses(X const& a, typename X::key_type v)
        -> void
{
        auto x = a;
        check_refuses([&] -> void { x |= v; });
        check_refuses([&] -> void { x ^= v; });
        check_refuses([&] -> void { static_cast<void>(x | v); });
        check_refuses([&] -> void { static_cast<void>(v ^ x); });
        check_refuses([&] -> void { static_cast<void>(v - x); });
        BOOST_CHECK(x == a);
        auto const met = a & v;
        BOOST_CHECK(met.empty() and (a - v) == a);
}

// An enumerator is a one-element set, and each operator meets a key as the model's set algebra does.
template<class X, class Model>
auto meets_it_as_a_one_element_set(X const& a, Model const& model, typename X::key_type v)
        -> void
{
        auto const held = model.contains(v);
        auto with       = model;
        with.insert(v);
        auto without = model;
        without.erase(v);
        auto x = a;
        x |= v;
        BOOST_CHECK(modelled<Model>(x) == with);
        auto y = a;
        y ^= v;
        BOOST_CHECK(modelled<Model>(y) == (held ? without : with));
        auto const joined   = a | v;
        auto const toggled  = v ^ a;
        auto const rest     = v - a;
        auto const met      = a & v;
        auto const remained = a - v;
        BOOST_CHECK(modelled<Model>(joined) == with);
        BOOST_CHECK(modelled<Model>(toggled) == (held ? without : with));
        BOOST_CHECK(modelled<Model>(rest) == (held ? Model{} : Model{v}));
        BOOST_CHECK(modelled<Model>(met) == (held ? Model{v} : Model{}));
        BOOST_CHECK(modelled<Model>(remained) == without);
}

} // namespace

// A key takes its position and a value that is no key has none, at a fixed width or a growing one; asking stays total.
BOOST_AUTO_TEST_CASE(EveryOwnerAdmitsAKeyAndRefusesAValueThatIsNoKey)
{
        test::for_each_type<closed_owners>([]<class X> -> void {
                using key_type    = X::key_type;
                using key_mapping = X::key_mapping_type;
                auto const keys   = universe<X>();
                auto const a      = X({keys.front(), keys.back()});
                auto const model  = std::set<key_type, typename X::key_compare>(a.begin(), a.end());
                for (auto const v : offered<key_type>()) {
                        BOOST_TEST_CONTEXT("value: " << int{std::to_underlying(v)})
                        {
                                BOOST_CHECK_EQUAL(test::set::lookup_mismatches(a, model, v), 0UZ);
                                if (key_mapping::is_key(v)) {
                                        admits_at_every_door(a, model, v);
                                        if constexpr (not xstd::bit_mask_mapping<key_mapping, key_type>) {
                                                meets_it_as_a_one_element_set(a, model, v);
                                        }
                                } else {
                                        refuses_at_every_door(a, v);
                                        if constexpr (not xstd::bit_mask_mapping<key_mapping, key_type>) {
                                                meets_it_as_an_empty_set_or_refuses(a, v);
                                        }
                                }
                        }
                }
        });
}

// The universe bounds every column alike: max_size() is its size, and a shift keeps only the keys that land in it.
BOOST_AUTO_TEST_CASE(EveryOwnerHoldsAtMostItsUniverse)
{
        test::for_each_type<closed_owners>([]<class X> -> void {
                using key_type    = X::key_type;
                using key_mapping = X::key_mapping_type;
                auto const keys   = universe<X>();
                auto x            = X(keys.begin(), keys.end());
                BOOST_CHECK_EQUAL(x.max_size(), key_mapping::size);
                BOOST_CHECK(x.full());
                x <<= 1UZ;
                BOOST_CHECK_EQUAL(x.size(), key_mapping::size - 1UZ);
                BOOST_CHECK(not x.contains(keys.front()));
                BOOST_CHECK(std::ranges::all_of(x, [](key_type k) -> bool { return key_mapping::is_key(k); }));
        });
}

// The two growths the reading computes by addition, over a size_t the caller names and bounded by no width.
BOOST_AUTO_TEST_CASE(TheGrowthsThatComputeAWidthSaturateRatherThanWrap)
{
        constexpr auto top = std::numeric_limits<std::size_t>::max();

        // The public way in first: a key past the top is one no width admits, whether or not n + 1 is itself a number.
        auto keyed = xstd::bit_set();
        keyed.insert(7UZ);
        BOOST_CHECK_THROW((void)keyed.insert(top), std::length_error);
        BOOST_CHECK_THROW((void)keyed.insert(top - 1UZ), std::length_error);
        BOOST_CHECK_EQUAL(keyed.size(), 1UZ);
        BOOST_CHECK(keyed.contains(7UZ));

        // The consecutive tier grows to admit the range's last position, which at the top of size_t has no width.
        auto ranged = xstd::bit_set();
        BOOST_CHECK_THROW(ranged.insert_range(std::views::iota(top - 2UZ, top)), std::length_error);
        BOOST_CHECK(ranged.empty());
}

// A left shift grows the width only towards max_size(), and a key it would carry past that is dropped, not refused.
BOOST_AUTO_TEST_CASE(ALeftShiftDropsTheKeysItCarriesPastMaxSize)
{
        constexpr auto top = std::numeric_limits<std::size_t>::max();
        auto shifted       = xstd::bit_set();
        shifted.insert(0UZ);
        shifted <<= 64UZ;
        BOOST_CHECK_EQUAL(shifted.size(), 1UZ);
        BOOST_CHECK(shifted.contains(64UZ));
        for (auto const n : {shifted.max_size() - 64UZ, top - 64UZ, top}) {
                auto emptied = shifted;
                emptied <<= n;
                BOOST_CHECK(emptied.empty());
        }
}

// for_each is the block-at-a-time walk an iterator cannot be, so it must answer exactly what iteration answers.
BOOST_AUTO_TEST_CASE(ForEachVisitsWhatIterationVisits)
{
        auto const positions = {0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 99UZ};

        auto owner = Owner();
        for (auto const p : positions) {
                owner.insert(p);
        }

        auto blocked = Storage();
        for (auto const p : positions) {
                blocked.set(p);
        }
        auto const fv = View(blocked);

        auto const collect         = [](auto const& s) -> std::vector<std::size_t> { auto v = std::vector<std::size_t>(); s.for_each        ([&](std::size_t p) -> void { v.push_back(p); }); return v; };
        auto const collect_reverse = [](auto const& s) -> std::vector<std::size_t> { auto v = std::vector<std::size_t>(); s.for_each_reverse([&](std::size_t p) -> void { v.push_back(p); }); return v; };
        auto const iterated        = [](auto const& s) -> std::vector<std::size_t> { return {s.begin(), s.end()}; };
        auto const reversed        = [](auto const& s) -> std::vector<std::size_t> { return {s.rbegin(), s.rend()}; };

        BOOST_CHECK(collect(owner) == iterated(owner));
        BOOST_CHECK(collect(fv) == iterated(fv));

        BOOST_CHECK(collect_reverse(owner) == reversed(owner));
        BOOST_CHECK(collect_reverse(fv) == reversed(fv));

        // An empty set calls nothing, in either direction, owned or viewed.
        auto const empty  = Owner();
        auto bnone        = Storage();
        auto const bnonev = View(bnone);
        auto calls        = 0UZ;
        empty.for_each([&](std::size_t) -> void { ++calls; });
        empty.for_each_reverse([&](std::size_t) -> void { ++calls; });
        bnonev.for_each([&](std::size_t) -> void { ++calls; });
        bnonev.for_each_reverse([&](std::size_t) -> void { ++calls; });
        BOOST_CHECK_EQUAL(calls, 0UZ);
}

// A functor returning bool means "keep going", which is what a move generator wants once it has its answer.
BOOST_AUTO_TEST_CASE(ForEachStopsWhenTheFunctorSaysSo)
{
        auto const positions = {0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 99UZ};

        auto owner = Owner();
        for (auto const p : positions) {
                owner.insert(p);
        }
        auto viewed = Storage();
        for (auto const p : positions) {
                viewed.set(p);
        }
        auto const bv = View(viewed);

        // Stop after the first position at or above 64, so the cut lands on a block boundary.
        auto const upto = [](auto const& s) -> std::vector<std::size_t> {
                auto v = std::vector<std::size_t>();
                s.for_each([&](std::size_t p) -> bool { v.push_back(p); return p < 64UZ; });
                return v;
        };
        auto const expected = std::vector<std::size_t>{0UZ, 1UZ, 63UZ, 64UZ};
        BOOST_CHECK(upto(owner) == expected);
        BOOST_CHECK(upto(bv) == expected);

        auto const downto = [](auto const& s) -> std::vector<std::size_t> {
                auto v = std::vector<std::size_t>();
                s.for_each_reverse([&](std::size_t p) -> bool { v.push_back(p); return p > 64UZ; });
                return v;
        };
        auto const expected_reverse = std::vector<std::size_t>{99UZ, 65UZ, 64UZ};
        BOOST_CHECK(downto(owner) == expected_reverse);
        BOOST_CHECK(downto(bv) == expected_reverse);

        // Stopping at the very first position visits exactly one.
        auto first_only = 0UZ;
        owner.for_each([&](std::size_t) -> bool { ++first_only; return false; });
        BOOST_CHECK_EQUAL(first_only, 1UZ);
}

// The functor is handed the position by value, and the constraint says so.
BOOST_AUTO_TEST_CASE(ForEachHandsThePositionByValue)
{
        // By value, generic or not, and by const reference: all four read what they are given.
        static_assert(walks<Owner, decltype([](std::size_t) -> void {})>);
        static_assert(walks<Owner, decltype([](auto) -> void {})>);
        static_assert(walks<Owner, decltype([](std::size_t const&) -> void {})>);
        static_assert(walks<Owner, decltype([](auto const&) -> void {})>);

        // A plain function is a functor too, and stays one.
        static_assert(walks<Owner, void (*)(std::size_t)>);

        // A functor returning bool to mean "keep going" is the other accepted shape.
        static_assert(walks<Owner, decltype([](std::size_t) -> bool { return true; })>);

        // And the two that would have written to nothing.
        static_assert(not walks<Owner, decltype([](std::size_t&) -> void {})>);
        static_assert(not walks<Owner, decltype([](auto&) -> void {})>);

        // The mirror is constrained alike, and so is a view over foreign storage.
        static_assert(walks_reverse<Owner, decltype([](std::size_t) -> void {})>);
        static_assert(not walks_reverse<Owner, decltype([](std::size_t&) -> void {})>);
        static_assert(not walks<View, decltype([](std::size_t&) -> void {})>);

        // And the overload resolution the constraint cannot reach: an lvalue at the call would take the reference.
        auto owner = Owner();
        owner.insert(3UZ);
        auto took_a_reference = false;
        owner.for_each(void_probe{took_a_reference});
        BOOST_CHECK(not took_a_reference);
        owner.for_each(bool_probe{took_a_reference});
        BOOST_CHECK(not took_a_reference);
        owner.for_each_reverse(void_probe{took_a_reference});
        BOOST_CHECK(not took_a_reference);
        owner.for_each_reverse(bool_probe{took_a_reference});
        BOOST_CHECK(not took_a_reference);
}

// back() has a non-empty set as its precondition, and a zero width has no non-empty state to ask it in.
BOOST_AUTO_TEST_CASE(AZeroWidthAnswersBackWithoutScanning)
{
        auto const z = xstd::bit_fixed_set<0>();

        BOOST_CHECK(z.empty());
        BOOST_CHECK_EQUAL(z.size(), 0UZ);
        BOOST_CHECK_EQUAL(static_cast<std::size_t>(z.back()), 0UZ);
}

// Across two run-time widths the set operations ask whole blocks, with padding clear above them by invariant.
namespace {

[[nodiscard]] auto grown_to(std::size_t width, std::initializer_list<std::size_t> positions)
        -> xstd::bit_set
{
        auto s = xstd::bit_set();
        s.insert(width);
        s.erase(width);
        for (auto const p : positions) {
                s.insert(p);
        }
        return s;
}

// The width is capacity: an owner reports max_size() as everything it could grow to, a view the storage's own size.
[[nodiscard]] auto width_of(xstd::bit_set& s)
        -> std::size_t
{
        return xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_block_container<std::vector<std::size_t>>, xstd::bits::detail::storage::borrowed>(s).max_size();
}

} // namespace

// A left shift that keeps no key empties the set at the width it has, rather than grow for an empty result.
BOOST_AUTO_TEST_CASE(ALeftShiftThatKeepsNoKeyKeepsTheWidth)
{
        auto s           = grown_to(300UZ, {1UZ, 5UZ, 59UZ});
        auto const width = width_of(s);
        s <<= s.max_size() - 1UZ;
        BOOST_CHECK(s.empty());
        BOOST_CHECK_EQUAL(width_of(s), width);

        // An empty set has no key to keep, so even a distance a width could hold leaves the width as it is.
        auto e = grown_to(10UZ, {});
        e <<= 100UZ;
        BOOST_CHECK(e.empty());
        BOOST_CHECK_EQUAL(width_of(e), 11UZ);
}

BOOST_AUTO_TEST_CASE(EqualityAcrossWidthsComparesBlocks)
{
        auto const one_block   = grown_to(60UZ, {1UZ, 5UZ, 59UZ});  // width 61
        auto const one_block_2 = grown_to(63UZ, {1UZ, 5UZ, 59UZ});  // width 64: same block count, other width
        auto const five_blocks = grown_to(300UZ, {1UZ, 5UZ, 59UZ}); // width 301

        // Widths differ but the block counts agree, so there is no remainder to look at.
        BOOST_CHECK(one_block == one_block_2);
        BOOST_CHECK(one_block_2 == one_block);

        // A remainder that is clear, both operand orders, the wider one carrying it either side.
        BOOST_CHECK(one_block == five_blocks);
        BOOST_CHECK(five_blocks == one_block);

        // A position living only in the remainder is what the remainder check is for.
        auto const with_high = grown_to(300UZ, {1UZ, 5UZ, 59UZ, 280UZ});
        BOOST_CHECK(one_block != with_high);
        BOOST_CHECK(with_high != one_block);

        // A difference inside the shared blocks, above the narrower width but below its last block's end.
        auto const with_near = grown_to(300UZ, {1UZ, 5UZ, 59UZ, 62UZ});
        BOOST_CHECK(one_block != with_near);
        BOOST_CHECK(with_near != one_block);
}

BOOST_AUTO_TEST_CASE(SubsetAndIntersectionAcrossWidthsCompareBlocks)
{
        auto const narrow = grown_to(60UZ, {1UZ, 5UZ});
        auto const wide   = grown_to(300UZ, {1UZ, 5UZ, 59UZ, 280UZ});

        // Ours inside theirs: nothing of ours lies above their last block, so the remainder is empty.
        BOOST_CHECK(narrow.is_subset_of(wide));

        // Theirs is not inside ours: 280 lives above our last block, which the remainder check catches.
        BOOST_CHECK(not wide.is_subset_of(narrow));

        // A wider operand whose remainder IS clear still fails on the shared blocks when it holds more there.
        auto const wide_low = grown_to(300UZ, {1UZ, 5UZ, 59UZ});
        BOOST_CHECK(not wide_low.is_subset_of(narrow));
        BOOST_CHECK(narrow.is_subset_of(wide_low));

        // And succeeds when the shared blocks agree and the remainder is clear.
        auto const wide_same = grown_to(300UZ, {1UZ, 5UZ});
        BOOST_CHECK(wide_same.is_subset_of(narrow));

        // Meeting in a shared block, and not meeting at all.
        BOOST_CHECK(intersects(narrow, wide));
        BOOST_CHECK(intersects(wide, narrow));

        auto const elsewhere = grown_to(300UZ, {7UZ, 280UZ});
        BOOST_CHECK(not intersects(narrow, elsewhere));
        BOOST_CHECK(not intersects(elsewhere, narrow));
}

// The ordering turns on one position, the lowest at which the two sets disagree, in both operand orders.
BOOST_AUTO_TEST_CASE(OrderingAcrossWidthsComparesBlocks)
{
        auto const narrow = [](std::initializer_list<std::size_t> p) -> xstd::bit_set { return grown_to(60UZ, p); };  // width 61, one block
        auto const wide   = [](std::initializer_list<std::size_t> p) -> xstd::bit_set { return grown_to(300UZ, p); }; // width 301, five blocks

        // Equal contents at different widths: no differing block at all.
        BOOST_CHECK((narrow({1UZ, 5UZ}) <=> wide({1UZ, 5UZ})) == std::strong_ordering::equal);

        // The lowest difference is held by the left, and the right has something above it to be smaller with.
        BOOST_CHECK((narrow({1UZ, 5UZ}) <=> wide({1UZ, 7UZ})) == std::strong_ordering::less);
        BOOST_CHECK((wide({1UZ, 7UZ}) <=> narrow({1UZ, 5UZ})) == std::strong_ordering::greater);

        // The lowest difference is held by the left, and the right stops there: a proper prefix, so the right is less.
        BOOST_CHECK((narrow({1UZ, 5UZ}) <=> wide({1UZ})) == std::strong_ordering::greater);
        BOOST_CHECK((wide({1UZ}) <=> narrow({1UZ, 5UZ})) == std::strong_ordering::less);

        // The position above lives in a later block, which is the other half of the look upward.
        BOOST_CHECK((narrow({1UZ, 5UZ}) <=> wide({1UZ, 200UZ})) == std::strong_ordering::less);

        // The difference itself lives past the narrower storage's last block, where its blocks read as zero.
        BOOST_CHECK((narrow({1UZ, 5UZ}) <=> wide({1UZ, 5UZ, 280UZ})) == std::strong_ordering::less);
        BOOST_CHECK((wide({1UZ, 5UZ, 280UZ}) <=> narrow({1UZ, 5UZ})) == std::strong_ordering::greater);

        // Two widths sharing a block count still differ, so this is the width-crossing arm and not the storage's.
        BOOST_CHECK((grown_to(60UZ, {1UZ, 5UZ}) <=> grown_to(63UZ, {1UZ, 7UZ})) == std::strong_ordering::less);

        // And the ordering agrees with the sets' own, which is what it is defined to be.
        BOOST_CHECK(narrow({1UZ, 5UZ}) < wide({1UZ, 7UZ}));
        BOOST_CHECK(wide({1UZ}) < narrow({1UZ, 5UZ}));
}

// The four compound operators blockwise: intersection and difference never widen, union and symmetric difference do.
BOOST_AUTO_TEST_CASE(TheCompoundOperatorsAcrossWidthsWorkOnBlocks)
{
        auto const narrow = grown_to(60UZ, {1UZ, 5UZ, 59UZ});    // width 61
        auto const wide   = grown_to(300UZ, {5UZ, 59UZ, 280UZ}); // width 301

        // Intersection keeps the shared positions at its own width, the wider operand's 280 having nowhere to land.
        auto a = narrow;
        a &= wide;
        BOOST_CHECK(a == grown_to(60UZ, {5UZ, 59UZ}));
        BOOST_CHECK_EQUAL(width_of(a), 61UZ);

        // The same the other way round: the narrow operand's blocks read as zero above its width, so 280 goes.
        auto b = wide;
        b &= narrow;
        BOOST_CHECK(b == grown_to(60UZ, {5UZ, 59UZ}));
        BOOST_CHECK_EQUAL(width_of(b), 301UZ);

        // Difference never widens either, and a position above this width is one it cannot hold to begin with.
        auto c = narrow;
        c -= wide;
        BOOST_CHECK(c == grown_to(60UZ, {1UZ}));
        BOOST_CHECK_EQUAL(width_of(c), 61UZ);

        // Union widens to one past 280 -- not to 301, the wider operand's own width.
        auto d = narrow;
        d |= wide;
        BOOST_CHECK(d == grown_to(280UZ, {1UZ, 5UZ, 59UZ, 280UZ}));
        BOOST_CHECK_EQUAL(width_of(d), 281UZ);

        // Symmetric difference widens by the same rule, and cancels what the two share.
        auto e = narrow;
        e ^= wide;
        BOOST_CHECK(e == grown_to(280UZ, {1UZ, 280UZ}));
        BOOST_CHECK_EQUAL(width_of(e), 281UZ);

        // A wider operand holding nothing above this width widens nothing, however wide it is.
        auto f = narrow;
        f |= grown_to(300UZ, {7UZ});
        BOOST_CHECK(f == grown_to(60UZ, {1UZ, 5UZ, 7UZ, 59UZ}));
        BOOST_CHECK_EQUAL(width_of(f), 61UZ);

        // An empty operand is the same statement with nothing to look at, and must not widen either.
        auto g = narrow;
        g |= grown_to(300UZ, {});
        BOOST_CHECK(g == narrow);
        BOOST_CHECK_EQUAL(width_of(g), 61UZ);

        // Two widths sharing a block count still differ, so this is the width-crossing arm and not the storage's.
        auto h = grown_to(60UZ, {1UZ, 5UZ});
        h |= grown_to(63UZ, {7UZ});
        BOOST_CHECK(h == grown_to(60UZ, {1UZ, 5UZ, 7UZ}));
        BOOST_CHECK_EQUAL(width_of(h), 61UZ);
}

// A set view converts too, reaching the storage through the accessor rather than the member, as a Bits* requires.
BOOST_AUTO_TEST_CASE(ASetViewExchangesThroughTheBitsItRefersTo)
{
        // The file's own Storage and views: bit_set_view<Storage> is set_adaptor<Storage, refers>, which View spells.
        constexpr auto N = Storage::extent;

        static_assert(test::converts_to<View, std::bitset<N>>);
        static_assert(test::converts_to<Reader, std::bitset<N>>);

        auto storage    = Storage();
        auto const view = View(storage);
        view.insert(0UZ);
        view.insert(31UZ);
        view.insert(N - 1UZ);

        auto const out = xstd::bit_convert<std::bitset<N>>(view);
        BOOST_CHECK_EQUAL(out.count(), 3UZ);
        BOOST_CHECK(out.test(0) and out.test(31) and out.test(N - 1UZ));

        // A view is built from what it views, never from a field of bits, so the inbound direction is the owner's.
        static_assert(not test::converts_from<View, std::bitset<N>>);

        // And at compile time, which is where the hard error would have been loudest.
        static_assert([] -> bool {
                auto bits    = Storage();
                auto const v = View(bits);
                v.insert(7UZ);
                return xstd::bit_convert<std::bitset<N>>(v).count() == 1UZ;
        }());
}

namespace {

template<class Compare, std::size_t N>
using fixed_by = xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, N, xstd::bit_key_mapping<std::size_t>, Compare>;

template<class Compare>
using dynamic_by = xstd::basic_bit_set<std::size_t, std::uint8_t, xstd::bit_key_mapping<std::size_t>, Compare>;

using descending_fixed = fixed_by<std::greater<std::size_t>, 130>;

// Dependent, so a comparator the owners reject is a false rather than a hard error.
template<class Compare>
constexpr bool fixed_takes = requires { typename xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 8, xstd::bit_key_mapping<std::size_t>, Compare>; }; // NOLINT(readability-redundant-typename): a type-requirement is spelled with it

template<class Compare>
constexpr bool bounded_takes = requires { typename xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 8, xstd::bit_key_mapping<std::size_t>, Compare>; }; // NOLINT(readability-redundant-typename): a type-requirement is spelled with it

template<class Compare>
constexpr bool small_takes = requires { typename xstd::basic_bit_small_set<std::size_t, std::uint8_t, 8, xstd::bit_key_mapping<std::size_t>, Compare>; }; // NOLINT(readability-redundant-typename): a type-requirement is spelled with it

template<class Compare>
constexpr bool dynamic_takes = requires { typename xstd::basic_bit_set<std::size_t, std::uint8_t, xstd::bit_key_mapping<std::size_t>, Compare>; }; // NOLINT(readability-redundant-typename): a type-requirement is spelled with it

template<class Compare>
constexpr bool every_owner_takes = fixed_takes<Compare> and bounded_takes<Compare> and small_takes<Compare> and dynamic_takes<Compare>;

template<class Compare>
constexpr bool no_owner_takes = not fixed_takes<Compare> and not bounded_takes<Compare> and not small_takes<Compare> and not dynamic_takes<Compare>;

// A set's mask as 64-bit words, lowest first, whatever its block.
template<class Set>
[[nodiscard]] auto words_of(Set const& s, std::size_t width)
        -> std::vector<std::uint64_t>
{
        auto words = std::vector<std::uint64_t>((width + 63UZ) / 64UZ);
        for (std::size_t const k : s) {
                words[k / 64UZ] |= std::uint64_t{1} << (k % 64UZ);
        }
        return words;
}

// The masks as unsigned numbers: the highest word decides first.
[[nodiscard]] auto numeric_order(std::vector<std::uint64_t> const& x, std::vector<std::uint64_t> const& y)
        -> std::strong_ordering
{
        return std::lexicographical_compare_three_way(x.rbegin(), x.rend(), y.rbegin(), y.rend());
}

// Each key below the width kept with probability density / 4.
template<class Set>
[[nodiscard]] auto drawn(test::spec::random::engine& engine, std::size_t width, std::uint64_t density)
        -> Set
{
        auto s = Set();
        for (auto const k : std::views::iota(0UZ, width)) {
                if (engine.below(4U) < density) {
                        s.insert(k);
                }
        }
        return s;
}

// A key that stands for every key in one decade, so its equivalence class spans ten positions.
struct decade
{
        std::size_t tens;

        [[nodiscard]] friend constexpr auto operator<=>(decade const& lhs, std::size_t rhs) noexcept
                -> std::strong_ordering
        {
                return lhs.tens <=> rhs / 10UZ;
        }

        [[nodiscard]] friend constexpr auto operator==(decade const& lhs, std::size_t rhs) noexcept
                -> bool
        {
                return lhs.tens == rhs / 10UZ;
        }
};

// Each heterogeneous member against std::set under the same transparent comparator, iterators as distances.
template<class Set, class Model, class K>
auto check_heterogeneous(Set const& s, Model const& model, K const& k)
        -> void
{
        auto const at = [](auto const& c, auto i) -> std::ptrdiff_t { return std::ranges::distance(c.begin(), i); };
        BOOST_CHECK_EQUAL(s.contains(k), model.contains(k));
        BOOST_CHECK_EQUAL(s.count(k), model.count(k));
        BOOST_CHECK_EQUAL(s.find(k) == s.end(), model.find(k) == model.end());
        if (auto const found = s.find(k); found != s.end()) {
                BOOST_CHECK(k == static_cast<std::size_t>(*found));
        }
        BOOST_CHECK_EQUAL(at(s, s.lower_bound(k)), at(model, model.lower_bound(k)));
        BOOST_CHECK_EQUAL(at(s, s.upper_bound(k)), at(model, model.upper_bound(k)));
        auto const [first, last] = s.equal_range(k);
        BOOST_CHECK(first == s.lower_bound(k) and last == s.upper_bound(k));

        // The model erases its equal range: not every standard library has P2077's heterogeneous erase on std::set.
        auto erased                          = s;
        auto erased_model                    = model;
        auto const [model_first, model_last] = erased_model.equal_range(k);
        erased_model.erase(model_first, model_last);
        BOOST_CHECK_EQUAL(erased.erase(k), model.count(k));
        BOOST_CHECK(std::ranges::equal(erased, erased_model));
}

template<class Compare, class Set>
auto check_transparent()
        -> void
{
        auto engine = test::spec::random::engine(test::spec::random::seed());
        for (auto const trial : std::views::iota(0UZ, 20UZ)) {
                auto const s     = drawn<Set>(engine, 100UZ, 1U + (trial % 3UZ));
                auto const model = std::set<std::size_t, Compare>(s.begin(), s.end());
                BOOST_CHECK(std::ranges::equal(s, model));
                for (auto const k : std::views::iota(0UZ, 102UZ)) {
                        check_heterogeneous(s, model, test::set::heterogeneous_key{.value = k});
                }
                for (auto const t : std::views::iota(0UZ, 12UZ)) {
                        check_heterogeneous(s, model, decade{.tens = t});
                }
        }
}

} // namespace

// Position order is structural: the comparators that pick a direction are taken, and nothing else is.
BOOST_AUTO_TEST_CASE(OnlyTheFourDirectionsAreComparators)
{
        static_assert(every_owner_takes<std::less<std::size_t>>);
        static_assert(every_owner_takes<std::less<>>);
        static_assert(every_owner_takes<std::greater<std::size_t>>);
        static_assert(every_owner_takes<std::greater<>>);

        static_assert(no_owner_takes<std::less_equal<std::size_t>>);
        static_assert(no_owner_takes<std::greater_equal<>>);
        static_assert(no_owner_takes<std::less<int>>);
        static_assert(no_owner_takes<std::ranges::less>);
        static_assert(no_owner_takes<decltype([](std::size_t x, std::size_t y) -> bool { return x < y; })>);
        BOOST_CHECK(true);
}

// The short aliases keep std::less, and key_comp() hands back whichever comparator the set is named with.
BOOST_AUTO_TEST_CASE(KeyCompIsTheComparatorTheSetIsNamedWith)
{
        static_assert(std::same_as<xstd::bit_fixed_set<8>::key_compare, std::less<std::size_t>>);
        static_assert(std::same_as<xstd::bit_set::key_compare, std::less<std::size_t>>);
        static_assert(std::same_as<descending_fixed::key_compare, std::greater<std::size_t>>);
        static_assert(std::same_as<descending_fixed::value_compare, std::greater<std::size_t>>);
        static_assert(std::same_as<decltype(fixed_by<std::greater<>, 8>().key_comp()), std::greater<>>);
        static_assert(std::same_as<decltype(dynamic_by<std::less<>>().value_comp()), std::less<>>);
        BOOST_CHECK(true);
}

// [associative.reqmts]' ordering over descending keys: the masks compared as unsigned numbers, highest block first.
BOOST_AUTO_TEST_CASE(ADescendingOrderIsTheMasksNumericOrder)
{
        using byte = fixed_by<std::greater<std::size_t>, 8>;
        BOOST_CHECK((byte{5UZ, 1UZ} <=> byte{3UZ}) == std::strong_ordering::greater);
        BOOST_CHECK((byte{4UZ, 3UZ} <=> byte{4UZ, 2UZ, 1UZ}) == std::strong_ordering::greater);
        BOOST_CHECK((byte{5UZ} <=> byte{5UZ, 1UZ}) == std::strong_ordering::less);

        auto engine       = test::spec::random::engine(test::spec::random::seed());
        auto const agrees = [&]<class Set>(std::size_t width) -> void {
                for (auto const trial : std::views::iota(0UZ, 200UZ)) {
                        auto const x = drawn<Set>(engine, width, 1U + (trial % 3UZ));
                        auto const y = trial % 5UZ == 0UZ ? x : drawn<Set>(engine, width, 1U + (trial % 2UZ));
                        BOOST_CHECK((x <=> y) == numeric_order(words_of(x, width), words_of(y, width)));
                        BOOST_CHECK((x <=> y) == std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()));
                }
        };
        agrees.template operator()<fixed_by<std::greater<std::size_t>, 0>>(0UZ);
        agrees.template operator()<fixed_by<std::greater<std::size_t>, 17>>(17UZ);
        agrees.template operator()<fixed_by<std::greater<std::size_t>, 64>>(64UZ);
        agrees.template operator()<descending_fixed>(130UZ);
        agrees.template operator()<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 17, xstd::bit_key_mapping<std::size_t>, std::greater<>>>(17UZ);
}

// Across two run-time widths a missing high block reads as zero, so the numbers line up at every position.
BOOST_AUTO_TEST_CASE(ADescendingOrderAcrossWidthsComparesAlignedBlocks)
{
        using descending = dynamic_by<std::greater<std::size_t>>;
        auto const grown = [](std::size_t width, std::initializer_list<std::size_t> positions) -> descending {
                auto s = descending();
                s.insert(width);
                s.erase(width);
                s.insert(positions);
                return s;
        };
        BOOST_CHECK((grown(10UZ, {1UZ, 5UZ}) <=> grown(300UZ, {1UZ, 5UZ})) == std::strong_ordering::equal);
        BOOST_CHECK((grown(10UZ, {1UZ, 5UZ}) <=> grown(300UZ, {200UZ})) == std::strong_ordering::less);
        BOOST_CHECK((grown(300UZ, {200UZ}) <=> grown(10UZ, {1UZ, 5UZ})) == std::strong_ordering::greater);
        BOOST_CHECK((grown(10UZ, {7UZ}) <=> grown(300UZ, {1UZ, 5UZ})) == std::strong_ordering::greater);
}

// for_each walks in the iteration order and for_each_reverse against it, whichever the direction.
BOOST_AUTO_TEST_CASE(ForEachFollowsTheComparator)
{
        auto const s  = descending_fixed{0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 129UZ};
        auto forward  = std::vector<std::size_t>();
        auto backward = std::vector<std::size_t>();
        s.for_each([&](std::size_t k) -> void { forward.push_back(k); });
        s.for_each_reverse([&](std::size_t k) -> void { backward.push_back(k); });
        BOOST_CHECK(forward == (std::vector<std::size_t>{s.begin(), s.end()}));
        BOOST_CHECK(backward == (std::vector<std::size_t>{s.rbegin(), s.rend()}));
        BOOST_CHECK(forward == (std::vector<std::size_t>{129UZ, 65UZ, 64UZ, 63UZ, 1UZ, 0UZ}));
}

// A view reads the positions it refers to in ascending order, whatever the owner's comparator.
BOOST_AUTO_TEST_CASE(AViewOverADescendingOwnerAscends)
{
        auto owner      = descending_fixed{3UZ, 70UZ, 129UZ};
        auto const view = xstd::bit_set_view(owner);
        static_assert(std::same_as<decltype(view)::key_compare, std::less<std::size_t>>);
        BOOST_CHECK(std::ranges::equal(view, std::vector<std::size_t>{3UZ, 70UZ, 129UZ}));
        BOOST_CHECK(std::ranges::equal(owner, std::vector<std::size_t>{129UZ, 70UZ, 3UZ}));
}

// The guides carry a comparator through, as std::set's do.
BOOST_AUTO_TEST_CASE(TheGuidesDeduceTheComparator)
{
        auto const keys = std::vector<std::size_t>{2UZ, 9UZ, 4UZ};
        auto const s    = xstd::basic_bit_set(keys.begin(), keys.end(), std::greater<std::size_t>()); // NOLINT(modernize-use-transparent-functors): the key-typed form is the one deduced
        static_assert(std::same_as<decltype(s), xstd::basic_bit_set<std::size_t, std::size_t, xstd::bit_key_mapping<std::size_t>, std::greater<std::size_t>> const>);
        BOOST_CHECK(std::ranges::equal(s, std::vector<std::size_t>{9UZ, 4UZ, 2UZ}));
}

// The transparent directions find, count, bound and erase a key of another type, as std::set does.
BOOST_AUTO_TEST_CASE(TheTransparentDirectionsLookUpHeterogeneously)
{
        check_transparent<std::less<>, fixed_by<std::less<>, 100>>();
        check_transparent<std::greater<>, fixed_by<std::greater<>, 100>>();
        check_transparent<std::less<>, dynamic_by<std::less<>>>();
        check_transparent<std::greater<>, dynamic_by<std::greater<>>>();

        // A key past what the set holds, and a set with no positions at all.
        auto const narrow = dynamic_by<std::greater<>>{3UZ};
        BOOST_CHECK(not narrow.contains(test::set::heterogeneous_key{.value = 50UZ}));
        BOOST_CHECK(narrow.lower_bound(test::set::heterogeneous_key{.value = 50UZ}) == narrow.begin());
        auto const none = dynamic_by<std::less<>>();
        BOOST_CHECK(not none.contains(decade{.tens = 0UZ}));
}

namespace {

// The subset of the first width positions that mask picks.
template<class X>
auto subset_of(std::size_t mask, std::size_t width)
        -> X
{
        auto nrv = X();
        for (auto const i : std::views::iota(0UZ, width)) {
                if (((mask >> i) & 1UZ) != 0UZ) {
                        nrv.insert(i);
                }
        }
        return nrv;
}

// How often the queries disagree on one pair, with each other and with std::ranges::includes.
template<class X>
auto query_mismatches(X const& x, X const& y)
        -> std::size_t
{
        auto const all_of = x.is_superset_of(y);
        auto const proper = x.is_proper_superset_of(y);
        auto mismatches   = static_cast<std::size_t>(disjoint(x, y) == intersects(x, y));
        mismatches += static_cast<std::size_t>(all_of != y.is_subset_of(x) or all_of != std::ranges::includes(x, y, x.key_comp()));
        mismatches += static_cast<std::size_t>(proper != y.is_proper_subset_of(x) or proper != (all_of and not std::ranges::equal(x, y)));
        return mismatches;
}

// Every pair of subsets of the first width positions, owned and, where the owner can be viewed, viewed.
template<class X>
auto query_mismatches_over_pairs(std::size_t width)
        -> std::size_t
{
        auto mismatches = 0UZ;
        for (auto const lhs : std::views::iota(0UZ, 1UZ << width)) {
                auto const x = subset_of<X>(lhs, width);
                for (auto const rhs : std::views::iota(0UZ, 1UZ << width)) {
                        auto const y = subset_of<X>(rhs, width);
                        mismatches += query_mismatches(x, y);
                        if constexpr (requires { xstd::bit_set_view(x); }) {
                                mismatches += query_mismatches(xstd::bit_set_view(x), xstd::bit_set_view(y));
                        }
                }
        }
        return mismatches;
}

using small_descending_fixed = xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 6, xstd::bit_key_mapping<std::size_t>, std::greater<>>;
using small_descending_set   = xstd::basic_bit_set<std::size_t, std::uint8_t, xstd::bit_key_mapping<std::size_t>, std::greater<>>;

} // namespace

// disjoint is none-of, and is_superset_of all-of in std::ranges::includes's order, over every pair at every storage.
BOOST_AUTO_TEST_CASE(DisjointIsNotIntersectsAndTheSupersetIsTheSubsetReadTheOtherWay)
{
        for (auto const width : std::views::iota(0UZ, 7UZ)) {
                BOOST_CHECK_EQUAL(query_mismatches_over_pairs<xstd::bit_fixed_set<6>>(width), 0UZ);
                BOOST_CHECK_EQUAL(query_mismatches_over_pairs<small_descending_fixed>(width), 0UZ);
                BOOST_CHECK_EQUAL(query_mismatches_over_pairs<xstd::bit_bounded_set<6>>(width), 0UZ);
                BOOST_CHECK_EQUAL(query_mismatches_over_pairs<xstd::bit_set>(width), 0UZ);
                BOOST_CHECK_EQUAL(query_mismatches_over_pairs<small_descending_set>(width), 0UZ);
                BOOST_CHECK_EQUAL(query_mismatches_over_pairs<xstd::bit_small_set<6>>(width), 0UZ);
        }

        // Two widths of one dynamic set, and the empty set, which every set is a superset of and disjoint from.
        auto const wide   = xstd::bit_set({1UZ, 100UZ});
        auto const narrow = xstd::bit_set({1UZ});
        auto const empty  = xstd::bit_set();
        BOOST_CHECK(wide.is_superset_of(narrow) and not narrow.is_superset_of(wide) and not disjoint(wide, narrow));
        BOOST_CHECK(wide.is_proper_superset_of(narrow) and not narrow.is_proper_superset_of(wide));
        BOOST_CHECK(narrow.is_superset_of(empty) and narrow.is_proper_superset_of(empty) and disjoint(narrow, empty));
        BOOST_CHECK(empty.is_superset_of(empty) and not empty.is_proper_superset_of(empty));
        static_assert(noexcept(disjoint(wide, narrow)) and noexcept(wide.is_superset_of(narrow)) and noexcept(wide.is_proper_superset_of(narrow)));
}

BOOST_AUTO_TEST_SUITE_END()
