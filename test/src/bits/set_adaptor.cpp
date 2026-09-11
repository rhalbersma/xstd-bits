//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/minimal_traits.hpp>                    // minimal_traits
#include <xstd/bits/bit_set.hpp>                      // bit_set
#include <xstd/bits/bit_set_view.hpp>                 // bit_set_view
#include <xstd/bits/bit_static_set.hpp>               // bit_static_set
#include <xstd/bits/detail/contiguous_bit_array.hpp>  // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/ext/boost/dynamic_bitset.hpp>     // bit_traits over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>               // bit_traits over std::bitset
#include <xstd/bits/ownership.hpp>                    // ownership
#include <xstd/bits/set_adaptor.hpp>                  // set_adaptor
#include <boost/dynamic_bitset.hpp>                   // dynamic_bitset
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                  // lexicographical_compare_three_way, ranges::equal
#include <bitset>                                     // bitset
#include <compare>                                    // strong_ordering
#include <concepts>                                   // copyable, equality_comparable, regular, totally_ordered
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint8_t, uint64_t
#include <initializer_list>                           // initializer_list
#include <limits>                                     // numeric_limits
#include <ranges>                                     // bidirectional_range, iota
#include <set>                                        // set
#include <vector>                                     // vector

namespace {

using Storage = xstd::detail::bits::contiguous_bit_array<std::uint64_t, 100>;
using Owner   = xstd::basic_bit_static_set<std::uint64_t, 100>;
using View    = xstd::set_adaptor<Storage, xstd::ownership::refers>;
using Reader  = xstd::set_adaptor<Storage const, xstd::ownership::refers>;
using Minimal = xstd::set_adaptor<Storage, xstd::ownership::refers, test::minimal_traits<Storage>>;

// Dependent, so an absent member is a false rather than a hard error.
template<class S> constexpr bool can_insert     = requires (S s) { s.insert(0UZ); };
template<class S> constexpr bool can_erase      = requires (S s) { s.erase(0UZ); };
template<class S> constexpr bool can_clear      = requires (S s) { s.clear(); };
template<class S> constexpr bool can_fill       = requires (S s) { s.fill(); s.complement(); s.complement(0UZ); };
template<class S> constexpr bool can_swap       = requires (S s) { s.swap(s); };
template<class S> constexpr bool has_complement = requires (S s) { ~s; s & s; };

// What for_each accepts, dependent so a rejected functor is a false rather than a hard error.
template<class S, class F> constexpr bool walks         = requires (S const& s, F f) { s.for_each(f); };
template<class S, class F> constexpr bool walks_reverse = requires (S const& s, F f) { s.for_each_reverse(f); };

// Functors overloaded on the value category. The constraint above cannot tell the two overloads apart -- both
// make the functor invocable with a size_t -- so these, and only these, are what pin the prvalue at the call:
// an lvalue at the call takes the && overload away and lands on the & one. One probe per arm, because
// invoke_continues splits on the return type and each arm calls the functor for itself.
struct void_probe
{
        bool& took_a_reference;

        auto operator()(std::size_t&&) const -> void {}
        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(std::size_t&) const -> void { took_a_reference = true; }
};

struct bool_probe
{
        bool& took_a_reference;

        auto operator()(std::size_t&&) const -> bool { return true; }
        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(std::size_t&) const -> bool { took_a_reference = true; return true; }
};

template<class Set>
[[nodiscard]] auto keys(Set const& s)
        -> std::set<std::size_t>
{
        return { s.begin(), s.end() };
}

// Every reading-level question a view can answer, against std::set answering the same one: the whole first, then each key.
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

        auto const [ first, last ] = s.equal_range(x);
        BOOST_CHECK(first == s.lower_bound(x) and last == s.upper_bound(x));
}

template<class Set>
auto check_reads(Set const& s, std::set<std::size_t> const& model, std::size_t width)
        -> void
{
        check_whole(s, model);
        for (auto x = 0UZ; x <= width + 1UZ; ++x) {
                check_key(s, model, x);
        }
}

}       // namespace

BOOST_AUTO_TEST_SUITE(SetAdaptor)

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
BOOST_AUTO_TEST_CASE(WritingIsGatedByTheTraitsNotByThisConst)
{
        static_assert(can_insert<View> and can_erase<View> and can_clear<View> and can_fill<View>);
        static_assert(can_insert<View const> and can_erase<View const> and can_clear<View const>);
        static_assert(can_insert<Owner> and can_swap<Owner> and has_complement<Owner>);

        static_assert(not can_insert<Reader> and not can_erase<Reader> and not can_clear<Reader> and not can_fill<Reader>);
        static_assert(not can_insert<Owner const> and not can_erase<Owner const> and not can_clear<Owner const>);
        static_assert(not can_insert<Minimal>);
        static_assert(not can_swap<View> and not has_complement<View>);
}

BOOST_AUTO_TEST_CASE(AViewWritesThroughToWhatItViews)
{
        auto c = Storage();
        auto const v = View(c);

        auto const [ it, inserted ] = v.insert(3UZ);
        BOOST_CHECK(inserted and *it == 3UZ and c.test(3));
        BOOST_CHECK(not v.insert(3UZ).second);
        BOOST_CHECK(v.emplace(5UZ).second and c.test(5));
        BOOST_CHECK(*v.emplace_hint(v.end(), 7UZ) == 7UZ and c.test(7));
        BOOST_CHECK(*v.insert(v.end(), 9UZ) == 9UZ);
        v.insert({ 11UZ, 13UZ });
        auto const some = std::vector<std::size_t>{ 17UZ, 19UZ };
        v.insert_range(some);
        auto const nothing = std::vector<std::size_t>();
        v.insert(nothing.begin(), nothing.end());
        BOOST_CHECK(keys(v) == std::set<std::size_t>({ 3, 5, 7, 9, 11, 13, 17, 19 }));

        BOOST_CHECK_EQUAL(v.erase(5UZ), 1UZ);
        BOOST_CHECK_EQUAL(v.erase(5UZ), 0UZ);
        BOOST_CHECK(*v.erase(v.find(7UZ)) == 9UZ);
        BOOST_CHECK(v.erase(v.find(11UZ), v.find(19UZ)) == v.find(19UZ));
        BOOST_CHECK(keys(v) == std::set<std::size_t>({ 3, 9, 19 }));

        v.complement(19UZ);
        v.complement(20UZ);
        BOOST_CHECK(keys(v) == std::set<std::size_t>({ 3, 9, 20 }));

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
        BOOST_CHECK(r.is_subset_of(r) and not r.is_proper_subset_of(r) and r.intersects(r));
}

BOOST_AUTO_TEST_CASE(TheViewsAnswerEveryReadOverEveryStorage)
{
        for (auto const& model : { std::set<std::size_t>{}, { 0UZ }, { 3UZ, 63UZ, 64UZ, 99UZ }, { 99UZ } }) {
                auto a = Storage();
                auto v = xstd::detail::bits::contiguous_bit_vector<std::uint64_t>(100UZ);
                auto s = std::bitset<100>();
                auto d = boost::dynamic_bitset<>(100UZ);
                for (auto const p : model) {
                        a.set(p);
                        v.set(p);
                        s.set(p);
                        d.set(p);
                }
                check_reads(View(a), model, 100UZ);
                check_reads(Minimal(a), model, 100UZ);
                check_reads(xstd::set_adaptor<xstd::detail::bits::contiguous_bit_vector<std::uint64_t>, xstd::ownership::refers>(v), model, 100UZ);
                check_reads(xstd::set_adaptor<std::bitset<100>, xstd::ownership::refers>(s), model, 100UZ);
                check_reads(xstd::set_adaptor<boost::dynamic_bitset<>, xstd::ownership::refers>(d), model, 100UZ);
        }
}

// max_size is the positions there are to hold: the width in the type, what an owner's storage can address, or what a
// view is looking at, none of which is the address space. [design.md#max-size-is-the-bits]
BOOST_AUTO_TEST_CASE(MaxSizeIsThePositionsThereAreToHold)
{
        auto storage = Storage();
        static_assert(Owner().max_size() == 100UZ);
        BOOST_CHECK_EQUAL(View(storage).max_size(), 100UZ);

        // An owner grows to what its storage can address, which is whole blocks of it and never the address space.
        using Heap = xstd::set_adaptor<xstd::detail::bits::contiguous_bit_vector<std::uint64_t>, xstd::ownership::owns>;
        BOOST_CHECK_EQUAL(Heap().max_size(), xstd::detail::bits::contiguous_bit_vector<std::uint64_t>().max_size());
        BOOST_CHECK_LT(Heap().max_size(), std::numeric_limits<std::size_t>::max());

        // A view cannot grow what it views, so its max_size is that width -- and filling it is what full() means.
        auto v = xstd::detail::bits::contiguous_bit_vector<std::uint64_t>(10UZ);
        auto const view = xstd::set_adaptor<xstd::detail::bits::contiguous_bit_vector<std::uint64_t>, xstd::ownership::refers>(v);
        BOOST_CHECK_EQUAL(view.max_size(), 10UZ);
        BOOST_CHECK(not view.full());
        view.fill();
        BOOST_CHECK(view.full());
        BOOST_CHECK_EQUAL(view.size(), 10UZ);

        // Boost's own width, read through the view over it.
        using Boost = xstd::set_adaptor<boost::dynamic_bitset<>, xstd::ownership::refers>;
        auto b = boost::dynamic_bitset<>(9UZ);
        BOOST_CHECK_EQUAL(Boost(b).max_size(), 9UZ);
}

// The set operations use the storage's members where it has them, and its bulk operators where it has not.
BOOST_AUTO_TEST_CASE(TheSetPredicatesAgreeAcrossStorages)
{
        auto a = std::bitset<9>();
        auto b = std::bitset<9>();
        auto e = std::bitset<9>();
        a.set(1);
        b.set(1);
        b.set(3);
        using S = xstd::set_adaptor<std::bitset<9>, xstd::ownership::refers>;
        auto const x = S(a);
        auto const y = S(b);

        BOOST_CHECK(x.is_subset_of(y) and not y.is_subset_of(x));
        BOOST_CHECK(x.is_proper_subset_of(y) and not x.is_proper_subset_of(x) and not y.is_proper_subset_of(x));
        BOOST_CHECK(x.intersects(y) and not x.intersects(S(e)));
        BOOST_CHECK(x != y and x < y);
        BOOST_CHECK((x <=> y) == std::strong_ordering::less);

        x.insert(3UZ);
        BOOST_CHECK(x == y and not x.is_proper_subset_of(y));
}

// The ordering invariant: the block-wise entry and the iterators agree, and the fallback is the invariant itself. [design.md#the-ordering-invariant]
BOOST_AUTO_TEST_CASE(TheOrderingIsTheLexicographicOrderOfTheKeys)
{
        auto const patterns = std::vector<std::set<std::size_t>>{ {}, { 0 }, { 1 }, { 0, 1 }, { 0, 1, 99 }, { 63, 64 }, { 64 }, { 99 } };
        for (auto const& p : patterns) {
                for (auto const& q : patterns) {
                        auto const x = Owner(p.begin(), p.end());
                        auto const y = Owner(q.begin(), q.end());
                        BOOST_CHECK((x <=> y) == std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end()));
                        BOOST_CHECK((x <=> y) == (p <=> q));

                        auto s = std::bitset<100>();
                        auto t = std::bitset<100>();
                        for (auto const i : p) { s.set(i); }
                        for (auto const i : q) { t.set(i); }
                        using S = xstd::set_adaptor<std::bitset<100>, xstd::ownership::refers>;
                        BOOST_CHECK((S(s) <=> S(t)) == (p <=> q));
                        BOOST_CHECK((S(s) == S(t)) == (p == q));
                }
        }
}

BOOST_AUTO_TEST_CASE(TheNonMemberFormsAreTheOwners)
{
        auto const x = Owner{ 1, 2, 3 };
        auto const y = Owner{ 2, 3, 4 };
        BOOST_CHECK(keys(x & y) == std::set<std::size_t>({ 2, 3 }));
        BOOST_CHECK(keys(x | y) == std::set<std::size_t>({ 1, 2, 3, 4 }));
        BOOST_CHECK(keys(x ^ y) == std::set<std::size_t>({ 1, 4 }));
        BOOST_CHECK(keys(x - y) == std::set<std::size_t>({ 1 }));
        BOOST_CHECK(keys(x << 1) == std::set<std::size_t>({ 2, 3, 4 }));
        BOOST_CHECK(keys(x >> 1) == std::set<std::size_t>({ 0, 1, 2 }));
        BOOST_CHECK((~x).size() == 97UZ and not (~x).contains(2UZ));

        auto z = x;
        z = { 5, 6 };
        BOOST_CHECK(keys(z) == std::set<std::size_t>({ 5, 6 }));
        BOOST_CHECK_EQUAL(erase_if(z, [](auto k) { return k == 5UZ; }), 1UZ);
        BOOST_CHECK(keys(z) == std::set<std::size_t>({ 6 }));
        swap(z, z);
        BOOST_CHECK(keys(z) == std::set<std::size_t>({ 6 }));
}

// insert_range takes a tier above the element-wise loop where it can, and the point of every case here is that
// the answer is the element-wise one. A block-wise fill has to mask its first and last block, so the boundaries
// are where it would go wrong: a range inside one block, a range spanning several, and one of each degenerate
// shape. [design.md#the-range-members]
BOOST_AUTO_TEST_CASE(RangedInsertionAgreesWithTheElementwiseLoop)
{
        constexpr auto N = 100UZ;

        // The consecutive tier, over every [lo, hi) the width admits.
        for (auto lo = 0UZ; lo <= N; ++lo) {
                for (auto hi = lo; hi <= N; ++hi) {
                        auto ranged = xstd::bit_static_set<N>();
                        ranged.insert_range(std::views::iota(lo, hi));

                        auto elementwise = xstd::bit_static_set<N>();
                        for (auto i = lo; i < hi; ++i) {
                                elementwise.insert(i);
                        }
                        BOOST_CHECK(ranged == elementwise);
                }
        }

        // It has to leave what lies outside the range alone, which a whole-block write would not.
        auto seeded = xstd::bit_static_set<N>();
        seeded.insert(0UZ); seeded.insert(70UZ); seeded.insert(99UZ);
        auto expected = seeded;
        seeded.insert_range(std::views::iota(10UZ, 65UZ));
        for (auto i = 10UZ; i < 65UZ; ++i) {
                expected.insert(i);
        }
        BOOST_CHECK(seeded == expected);

        // The set tier: a union, and equally the element-wise answer.
        auto lhs = xstd::bit_static_set<N>();
        lhs.insert(1UZ); lhs.insert(64UZ);
        auto rhs = xstd::bit_static_set<N>();
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
        for (auto i = 5UZ; i < 130UZ; ++i) {
                elementwise.insert(i);
        }
        BOOST_CHECK(ranged == elementwise);
        BOOST_CHECK_EQUAL(ranged.size(), 125UZ);

        // Appending a second, disjoint stretch grows it again and keeps the first.
        ranged.insert_range(std::views::iota(200UZ, 260UZ));
        for (auto i = 200UZ; i < 260UZ; ++i) {
                elementwise.insert(i);
        }
        BOOST_CHECK(ranged == elementwise);
}

// for_each is the block-at-a-time walk an iterator cannot be, so what has to be shown is that it answers exactly
// what iteration answers -- over a storage with block access and over one without, which takes the other arm.
// [design.md#the-set-for-each]
BOOST_AUTO_TEST_CASE(ForEachVisitsWhatIterationVisits)
{
        auto const positions = { 0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 99UZ };

        auto owner = Owner();
        for (auto const p : positions) { owner.insert(p); }

        auto foreign = std::bitset<100>();                      // block access through the trait
        auto boosted = boost::dynamic_bitset<>(100);            // no block access: the position-at-a-time arm
        for (auto const p : positions) { foreign.set(p); boosted.set(p); }
        auto const fv = xstd::bit_set_view(foreign);
        auto const bv = xstd::bit_set_view(boosted);

        auto const collect         = [](auto const& s) -> std::vector<std::size_t> { auto v = std::vector<std::size_t>(); s.for_each        ([&](std::size_t p) -> void { v.push_back(p); }); return v; };
        auto const collect_reverse = [](auto const& s) -> std::vector<std::size_t> { auto v = std::vector<std::size_t>(); s.for_each_reverse([&](std::size_t p) -> void { v.push_back(p); }); return v; };
        auto const iterated        = [](auto const& s) -> std::vector<std::size_t> { return { s.begin(), s.end() }; };
        auto const reversed        = [](auto const& s) -> std::vector<std::size_t> { return { s.rbegin(), s.rend() }; };

        BOOST_CHECK(collect(owner) == iterated(owner));
        BOOST_CHECK(collect(fv)    == iterated(fv));
        BOOST_CHECK(collect(bv)    == iterated(bv));

        BOOST_CHECK(collect_reverse(owner) == reversed(owner));
        BOOST_CHECK(collect_reverse(fv)    == reversed(fv));
        BOOST_CHECK(collect_reverse(bv)    == reversed(bv));

        // An empty set calls nothing, in either direction, on either arm.
        auto const empty  = Owner();
        auto const bnone  = boost::dynamic_bitset<>(100);
        auto const bnonev = xstd::bit_set_view(bnone);
        auto calls = 0UZ;
        empty .for_each        ([&](std::size_t) -> void { ++calls; });
        empty .for_each_reverse([&](std::size_t) -> void { ++calls; });
        bnonev.for_each        ([&](std::size_t) -> void { ++calls; });
        bnonev.for_each_reverse([&](std::size_t) -> void { ++calls; });
        BOOST_CHECK_EQUAL(calls, 0UZ);
}

// A functor returning bool means "keep going", which is what a move generator wants once it has its answer. A
// void one always continues. Both arms honour it.
BOOST_AUTO_TEST_CASE(ForEachStopsWhenTheFunctorSaysSo)
{
        auto const positions = { 0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 99UZ };

        auto owner = Owner();
        for (auto const p : positions) { owner.insert(p); }
        auto boosted = boost::dynamic_bitset<>(100);
        for (auto const p : positions) { boosted.set(p); }
        auto const bv = xstd::bit_set_view(boosted);

        // Stop after the first position at or above 64, so the cut lands on a block boundary.
        auto const upto = [](auto const& s) -> std::vector<std::size_t> {
                auto v = std::vector<std::size_t>();
                s.for_each([&](std::size_t p) -> bool { v.push_back(p); return p < 64UZ; });
                return v;
        };
        auto const expected = std::vector<std::size_t>{ 0UZ, 1UZ, 63UZ, 64UZ };
        BOOST_CHECK(upto(owner) == expected);
        BOOST_CHECK(upto(bv)    == expected);

        auto const downto = [](auto const& s) -> std::vector<std::size_t> {
                auto v = std::vector<std::size_t>();
                s.for_each_reverse([&](std::size_t p) -> bool { v.push_back(p); return p > 64UZ; });
                return v;
        };
        auto const expected_reverse = std::vector<std::size_t>{ 99UZ, 65UZ, 64UZ };
        BOOST_CHECK(downto(owner) == expected_reverse);
        BOOST_CHECK(downto(bv)    == expected_reverse);

        // Stopping at the very first position visits exactly one.
        auto first_only = 0UZ;
        owner.for_each([&](std::size_t) -> bool { ++first_only; return false; });
        BOOST_CHECK_EQUAL(first_only, 1UZ);
}

// The functor is handed the position by value, and the constraint says so. Without that a functor asking for
// size_t& binds to the walker's own local, and its write goes nowhere -- a walk reports positions and changes
// none, so it is not a lost write but a meaningless one. [design.md#the-functor-takes-a-value]
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
        // Both arms and both directions, each of which calls the functor for itself.
        auto owner = Owner();
        owner.insert(3UZ);
        auto took_a_reference = false;
        owner.for_each        (void_probe{ took_a_reference }); BOOST_CHECK(not took_a_reference);
        owner.for_each        (bool_probe{ took_a_reference }); BOOST_CHECK(not took_a_reference);
        owner.for_each_reverse(void_probe{ took_a_reference }); BOOST_CHECK(not took_a_reference);
        owner.for_each_reverse(bool_probe{ took_a_reference }); BOOST_CHECK(not took_a_reference);
}

BOOST_AUTO_TEST_SUITE_END()
