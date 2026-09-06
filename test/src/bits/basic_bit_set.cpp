//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <test/minimal_traits.hpp>                // minimal_traits
#include <xstd/bits/basic_bit_set.hpp>            // basic_bit_set
#include <xstd/bits/bit_static_set.hpp>           // bit_static_set
#include <xstd/bits/block_sequence.hpp>           // block_array, block_vector
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // bit_traits over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bit_traits over std::bitset
#include <xstd/bits/ownership.hpp>                // ownership
#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <algorithm>                              // lexicographical_compare_three_way, ranges::equal
#include <bitset>                                 // bitset
#include <compare>                                // strong_ordering
#include <concepts>                               // copyable, equality_comparable, regular, totally_ordered
#include <cstddef>                                // size_t
#include <cstdint>                                // uint8_t, uint64_t
#include <initializer_list>                       // initializer_list
#include <limits>                                 // numeric_limits
#include <ranges>                                 // bidirectional_range
#include <set>                                    // set
#include <vector>                                 // vector

namespace {

using Storage = xstd::block_array<std::uint64_t, 100>;
using Owner   = xstd::bit_static_set<100, std::uint64_t>;
using View    = xstd::basic_bit_set<Storage, xstd::ownership::refers>;
using Reader  = xstd::basic_bit_set<Storage const, xstd::ownership::refers>;
using Minimal = xstd::basic_bit_set<Storage, xstd::ownership::refers, test::minimal_traits<Storage>>;

// Dependent, so an absent member is a false rather than a hard error.
template<class S> constexpr bool can_insert     = requires (S s) { s.insert(0UZ); };
template<class S> constexpr bool can_erase      = requires (S s) { s.erase(0UZ); };
template<class S> constexpr bool can_clear      = requires (S s) { s.clear(); };
template<class S> constexpr bool can_fill       = requires (S s) { s.fill(); s.complement(); s.complement(0UZ); };
template<class S> constexpr bool can_swap       = requires (S s) { s.swap(s); };
template<class S> constexpr bool has_complement = requires (S s) { ~s; s & s; };

template<class Set>
[[nodiscard]] auto keys(Set const& s) -> std::set<std::size_t>
{
        return { s.begin(), s.end() };
}

// Every reading-level question a view can answer, against std::set answering the same one: the whole first, then each key.
template<class Set>
auto check_whole(Set const& s, std::set<std::size_t> const& model) -> void
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
auto check_key(Set const& s, std::set<std::size_t> const& model, std::size_t x) -> void
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
auto check_reads(Set const& s, std::set<std::size_t> const& model, std::size_t width) -> void
{
        check_whole(s, model);
        for (auto x = 0UZ; x <= width + 1UZ; ++x) {
                check_key(s, model, x);
        }
}

}       // namespace

BOOST_AUTO_TEST_SUITE(BasicBitSet)

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
                auto v = xstd::block_vector<std::uint64_t>(100UZ);
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
                check_reads(xstd::basic_bit_set<xstd::block_vector<std::uint64_t>, xstd::ownership::refers>(v), model, 100UZ);
                check_reads(xstd::basic_bit_set<std::bitset<100>, xstd::ownership::refers>(s), model, 100UZ);
                check_reads(xstd::basic_bit_set<boost::dynamic_bitset<>, xstd::ownership::refers>(d), model, 100UZ);
        }
}

// max_size is the width where the type carries one, and the last addressable position where it does not.
BOOST_AUTO_TEST_CASE(MaxSizeIsStaticWhereTheWidthIs)
{
        static_assert(Owner::max_size() == 100UZ);
        static_assert(View::max_size() == 100UZ);
        static_assert(xstd::basic_bit_set<xstd::block_vector<std::size_t>, xstd::ownership::refers>::max_size() == std::numeric_limits<std::size_t>::max() - 1UZ);
        static_assert(xstd::basic_bit_set<boost::dynamic_bitset<>, xstd::ownership::refers>::max_size() == std::numeric_limits<std::size_t>::max() - 1UZ);

        auto v = xstd::block_vector<std::uint64_t>(10UZ);
        auto const view = xstd::basic_bit_set<xstd::block_vector<std::uint64_t>, xstd::ownership::refers>(v);
        BOOST_CHECK(not view.full());
        view.fill();
        BOOST_CHECK(not view.full());
        BOOST_CHECK_EQUAL(view.size(), 10UZ);
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
        using S = xstd::basic_bit_set<std::bitset<9>, xstd::ownership::refers>;
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
                        using S = xstd::basic_bit_set<std::bitset<100>, xstd::ownership::refers>;
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

BOOST_AUTO_TEST_SUITE_END()
