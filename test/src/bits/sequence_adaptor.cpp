//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <xstd/bits/sequence_adaptor.hpp>  // sequence_adaptor
#include <xstd/bits/bit_array.hpp>           // bit_array
#include <xstd/bits/block_sequence.hpp>      // block_array, block_vector
#include <xstd/bits/ext/std/bitset.hpp>      // bit_traits over std::bitset
#include <xstd/bits/ownership.hpp>           // ownership
#include <algorithm>                         // lexicographical_compare_three_way, ranges::equal
#include <bitset>                            // bitset
#include <compare>                           // strong_ordering
#include <concepts>                          // copyable, equality_comparable, regular, same_as, totally_ordered
#include <cstddef>                           // size_t
#include <cstdint>                           // uint64_t
#include <iterator>                          // reverse_iterator
#include <limits>                            // numeric_limits
#include <ranges>                            // random_access_range
#include <stdexcept>                         // out_of_range
#include <type_traits>                       // is_const_v
#include <vector>                            // vector

namespace {

using Storage = xstd::block_array<std::uint64_t, 100>;
using Owner   = xstd::basic_bit_array<100, std::uint64_t>;
using View    = xstd::sequence_adaptor<Storage, xstd::ownership::refers, false>;
using Reader  = xstd::sequence_adaptor<Storage const, xstd::ownership::refers, false>;

// Dependent, so an absent member is a false rather than a hard error.
template<class S> constexpr bool can_fill  = requires (S s) { s.fill(true); };
template<class S> constexpr bool can_write = requires (S s) { s[0] = true; };
template<class S> constexpr bool can_swap  = requires (S s) { s.swap(s); };

template<class Seq>
[[nodiscard]] auto bools(Seq const& s) -> std::vector<bool>
{
        return { s.begin(), s.end() };
}

}       // namespace

BOOST_AUTO_TEST_SUITE(SequenceAdaptor)

BOOST_AUTO_TEST_CASE(AnOwnerIsRegularAndAViewIsCopyable)
{
        static_assert(std::regular<Owner>);
        static_assert(std::totally_ordered<Owner>);
        static_assert(std::ranges::random_access_range<Owner>);

        static_assert(std::copyable<View> and std::copyable<Reader>);
        static_assert(std::ranges::random_access_range<View> and std::ranges::random_access_range<Reader>);
        static_assert(not std::default_initializable<View>);

        // A view follows span: no equality and no ordering. [design.md#views-follow-their-precedent]
        static_assert(not std::equality_comparable<View>);
        static_assert(not std::three_way_comparable<View>);
}

// Deep const for the owner, shallow for the view: what each hands out says which.
BOOST_AUTO_TEST_CASE(ConstIsDeepForTheOwnerAndShallowForTheView)
{
        auto a = Owner();  // NOLINT(misc-const-correctness): the non-const overloads are what the decltypes below ask about
        auto const& ca = a;
        static_assert(std::same_as<decltype(a.begin()),  Owner::iterator>);
        static_assert(std::same_as<decltype(ca.begin()), Owner::const_iterator>);
        static_assert(std::same_as<decltype(a[0]),  Owner::reference>);
        static_assert(std::same_as<decltype(ca[0]), Owner::const_reference>);
        static_assert(std::same_as<decltype(ca.cbegin()), Owner::const_iterator>);

        auto c = Storage();
        View const v(c);
        static_assert(std::same_as<decltype(v.begin()), View::iterator>);
        static_assert(std::same_as<decltype(v[0]), View::reference>);
        static_assert(std::same_as<decltype(v.cbegin()), View::const_iterator>);
        static_assert(std::is_const_v<std::remove_reference_t<decltype(*Reader(c).begin()) const>>);

        static_assert(can_fill<View> and can_write<View> and can_fill<Owner> and can_write<Owner> and can_swap<Owner>);
        static_assert(not can_fill<Reader> and not can_write<Reader>);
        static_assert(not can_fill<Owner const> and not can_write<Owner const>);
        static_assert(not can_swap<View>);
}

BOOST_AUTO_TEST_CASE(AViewWritesThroughToWhatItViews)
{
        auto c = Storage();
        View const v(c);
        auto model = std::vector<bool>(100);

        v[3] = true;
        v.at(5) = true;
        v.front() = true;
        v.back() = true;
        model[3] = model[5] = model[0] = model[99] = true;
        BOOST_CHECK(bools(v) == model);
        BOOST_CHECK(c.test(3) and c.test(5) and c.test(0) and c.test(99));
        BOOST_CHECK_EQUAL(c.count(), 4UZ);

        v.fill(false);
        BOOST_CHECK(c.none());
}

BOOST_AUTO_TEST_CASE(AViewIsARangeInBothDirectionsAndAtThrowsPastTheEnd)
{
        auto c = Storage();
        c.set(99);
        View const v(c);

        BOOST_CHECK(*v.rbegin() == true and *v.crbegin() == true);
        BOOST_CHECK(v.rend() - v.rbegin() == 100);
        BOOST_CHECK(v.crend() - v.crbegin() == 100);
        BOOST_CHECK(v.cend() - v.cbegin() == 100);
        BOOST_CHECK_EQUAL(v.size(), 100UZ);
        BOOST_CHECK_EQUAL(v.max_size(), 100UZ);
        BOOST_CHECK(not v.empty());

        // Named rather than a temporary: clang 23's lifetime analysis crashes on a deducing-this call with an rvalue self.
        auto const r = Reader(c);
        BOOST_CHECK_THROW(static_cast<void>(v.at(100)), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(r.at(100)), std::out_of_range);
        BOOST_CHECK(r.at(99) == true);
}

BOOST_AUTO_TEST_CASE(TheBulkOperatorsAreTheStoragesOwn)
{
        auto x = Owner();
        auto y = Owner();
        x[1] = x[2] = true;
        y[2] = y[3] = true;

        auto z = x;
        z &= y;
        BOOST_CHECK(z[2] and not z[1] and not z[3]);
        z = x;
        z |= y;
        BOOST_CHECK(z[1] and z[2] and z[3]);
        z = x;
        z ^= y;
        BOOST_CHECK(z[1] and not z[2] and z[3]);
        z = x;
        z -= y;
        BOOST_CHECK(z[1] and not z[2]);
        z = x;
        z <<= 1;
        BOOST_CHECK(z[2] and z[3] and not z[1]);
        z >>= 2;
        BOOST_CHECK(z[0] and z[1] and not z[2]);

        auto c = Storage();
        auto d = Storage();
        d.set(1);
        View const v(c);
        v |= View(d);
        BOOST_CHECK(c.test(1) and not c.test(2));
        swap(x, y);
        BOOST_CHECK(x[3] and y[1]);
}

// The ordering invariant on the trait's entry, the only ordering an owner has. [design.md#the-ordering-invariant]
BOOST_AUTO_TEST_CASE(TheOrderingIsTheLexicographicOrderOfTheBools)
{
        using Packed = xstd::basic_bit_array<9, std::uint8_t>;
        static_assert(std::regular<Packed> and std::totally_ordered<Packed>);

        // An owner over storage without the entry has no ordering rather than a synthesized one. [design.md#owning-is-ours]
        static_assert(not std::totally_ordered<xstd::sequence_adaptor<std::bitset<9>, xstd::ownership::owns, false>>);

        auto const patterns = std::vector<std::vector<std::size_t>>{ {}, { 0 }, { 1 }, { 0, 1 }, { 8 }, { 0, 8 } };
        for (auto const& p : patterns) {
                for (auto const& q : patterns) {
                        auto x = Packed();
                        auto y = Packed();
                        for (auto const i : p) { x[i] = true; }
                        for (auto const i : q) { y[i] = true; }

                        auto const expected = std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());
                        BOOST_CHECK((x <=> y) == expected);
                        BOOST_CHECK((x == y) == (p == q));
                }
        }
}

// Dependent, so a constrained-away member is a false rather than a hard error.
template<class X>
constexpr bool can_grow = requires (X& x) { x.push_back(true); x.pop_back(); x.resize(1UZ); x.resize(1UZ, true); x.clear(); x.reserve(1UZ); x.shrink_to_fit(); };

// Growth is the owner's over storage that grows; a static width and a view have none of it. [design.md#growth]
BOOST_AUTO_TEST_CASE(GrowthIsTheOwnersOverStorageThatGrows)
{
        using Dynamic = xstd::sequence_adaptor<xstd::block_vector<std::uint64_t>, xstd::ownership::owns, false>;
        using Span    = xstd::sequence_adaptor<xstd::block_vector<std::uint64_t>, xstd::ownership::refers, false>;

        static_assert(    can_grow<Dynamic>);
        static_assert(not can_grow<Owner>);
        static_assert(not can_grow<Span>);

        auto d = Dynamic(3, true);
        d.push_back(false);
        BOOST_CHECK_EQUAL(d.size(), 4UZ);
        BOOST_CHECK(std::ranges::equal(d, std::vector<bool>{ true, true, true, false }));
        BOOST_CHECK_EQUAL(d.max_size(), xstd::block_vector<std::uint64_t>().max_size());
        BOOST_CHECK_LT(d.max_size(), std::numeric_limits<std::size_t>::max());
        BOOST_CHECK_EQUAL(Owner().max_size(), 100UZ);
}

BOOST_AUTO_TEST_CASE(AZeroWidthSequenceIsEmpty)
{
        auto const a = xstd::basic_bit_array<0, std::uint8_t>();
        BOOST_CHECK(a.empty() and a.begin() == a.end());
        auto c = xstd::block_array<std::uint8_t, 0>();
        auto const v = xstd::sequence_adaptor<xstd::block_array<std::uint8_t, 0>, xstd::ownership::refers, false>(c);
        BOOST_CHECK(v.empty() and v.begin() == v.end());
}

BOOST_AUTO_TEST_SUITE_END()
