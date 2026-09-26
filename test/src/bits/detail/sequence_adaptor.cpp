//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bit_exchange.hpp>                         // casts_between, casts_from, exchanges_to_bits
#include <test/block_types.hpp>                          // graded_extents
#include <xstd/bits/bit/bit_cast.hpp>                    // bit_cast
#include <xstd/bits/bit_array.hpp>                       // bit_array
#include <xstd/bits/bit_sequence_adaptor.hpp>            // swap
#include <xstd/bits/bit_span.hpp>                        // bit_span
#include <xstd/bits/bit_subspan.hpp>                     // bit_subspan
#include <xstd/bits/bit_vector.hpp>                      // bit_vector
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // storage
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                     // all_of, any_of, count, equal, lexicographical_compare_three_way, mismatch, none_of
#include <array>                                         // array
#include <bitset>                                        // bitset
#include <compare>                                       // strong_ordering
#include <concepts>                                      // copyable, equality_comparable, regular, same_as, totally_ordered
#include <cstddef>                                       // ptrdiff_t, size_t
#include <cstdint>                                       // uint64_t
#include <iterator>                                      // reverse_iterator
#include <limits>                                        // numeric_limits
#include <ranges>                                        // equal, iota, random_access_range, transform
#include <span>                                          // dynamic_extent
#include <stdexcept>                                     // length_error, out_of_range
#include <type_traits>                                   // is_const_v, is_constructible_v, is_convertible_v
#include <utility>                                       // move, pair
#include <vector>                                        // vector

namespace {

using Storage = xstd::bits::detail::contiguous_bit_container<std::array<std::uint64_t, 2>, 100>;
using Owner = xstd::basic_bit_array<std::uint64_t, 100>;
using View = xstd::bits::detail::sequence_adaptor<Storage, xstd::bits::detail::storage::borrowed, xstd::bits::detail::window::all>;
using Reader = xstd::bits::detail::sequence_adaptor<Storage const, xstd::bits::detail::storage::borrowed, xstd::bits::detail::window::all>;

// Dependent, so an absent member is a false rather than a hard error.
template<class S>
constexpr bool can_fill = requires (S s) { s.fill(true); };
template<class S>
constexpr bool can_write = requires (S s) { s[0] = true; };
template<class S>
constexpr bool can_swap = requires (S s) { s.swap(s); };

template<class Seq>
[[nodiscard]] auto bools(Seq const& s)
        -> std::vector<bool>
{
        return {s.begin(), s.end()};
}

using DynamicOctet = xstd::bits::detail::sequence_adaptor<xstd::bits::detail::contiguous_bit_container<std::vector<std::uint8_t>>, xstd::bits::detail::storage::owned, xstd::bits::detail::window::all>;

// Every (size, pattern) pair as a sequence and the vector<bool> modelling it, so the comparison is one loop.
[[nodiscard]] auto dynamic_probes()
        -> std::vector<std::pair<DynamicOctet, std::vector<bool>>>
{
        auto const patterns = std::vector<std::vector<std::size_t>>{{}, {0}, {1}, {7}, {8}, {0, 8}, {7, 8}};
        auto out = std::vector<std::pair<DynamicOctet, std::vector<bool>>>();
        for (auto const n : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 16UZ, 17UZ}) {
                for (auto const& p : patterns) {
                        auto x = DynamicOctet(n, false);
                        auto v = std::vector<bool>(n, false);
                        for (auto const i : p) {
                                if (i < n) {
                                        x[i] = true;
                                        v[i] = true;
                                }
                        }
                        out.emplace_back(std::move(x), std::move(v));
                }
        }
        return out;
}

// Named so each requirement is checked on a template parameter: a deleted overload is a hard error otherwise.
template<class T>
concept eq_comparable = requires (T a, T b) { a == b; };
template<class T>
concept ne_comparable = requires (T a, T b) { a != b; };
template<class T>
concept spaceship_comparable = requires (T a, T b) { a <=> b; };
template<class T>
concept lt_comparable = requires (T a, T b) { a < b; };
template<class T>
concept gt_comparable = requires (T a, T b) { a > b; };
template<class T>
concept le_comparable = requires (T a, T b) { a <= b; };
template<class T>
concept ge_comparable = requires (T a, T b) { a >= b; };

} // namespace

BOOST_AUTO_TEST_SUITE(SequenceAdaptor)

BOOST_AUTO_TEST_CASE(AnOwnerIsRegularAndAViewIsCopyable)
{
        static_assert(std::regular<Owner>);
        static_assert(std::totally_ordered<Owner>);
        static_assert(std::ranges::random_access_range<Owner>);

        static_assert(std::copyable<View> and std::copyable<Reader>);
        static_assert(std::ranges::random_access_range<View> and std::ranges::random_access_range<Reader>);
        static_assert(not std::default_initializable<View>);

        // A view follows span: no equality and no ordering.
        static_assert(not std::equality_comparable<View>);
        static_assert(not std::three_way_comparable<View>);

        // Spelled out: a partial deletion leaves a working subset, <=> rewriting the relationals and never ==.
        static_assert(not eq_comparable<View>);
        static_assert(not ne_comparable<View>);
        static_assert(not spaceship_comparable<View>);
        static_assert(not lt_comparable<View>);
        static_assert(not gt_comparable<View>);
        static_assert(not le_comparable<View>);
        static_assert(not ge_comparable<View>);

        // The owner answers all seven, so the assertions above are the view's shape and not a dead concept.
        static_assert(eq_comparable<Owner> and ne_comparable<Owner> and spaceship_comparable<Owner>);
        static_assert(lt_comparable<Owner> and gt_comparable<Owner> and le_comparable<Owner> and ge_comparable<Owner>);
}

// Deep const for the owner, shallow for the view: what each hands out says which.
BOOST_AUTO_TEST_CASE(ConstIsDeepForTheOwnerAndShallowForTheView)
{
        auto a = Owner(); // NOLINT(misc-const-correctness): the non-const overloads are what the decltypes below ask about
        auto const& ca = a;
        static_assert(std::same_as<decltype(a.begin()), Owner::iterator>);
        static_assert(std::same_as<decltype(ca.begin()), Owner::const_iterator>);
        static_assert(std::same_as<decltype(a[0]), Owner::reference>);
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

        // Named rather than a temporary: clang 23 crashes on a deducing-this call with an rvalue self.
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

        // The binary forms, each its compound over a copy, and the complement as flip()'s value.
        auto t = x;
        t &= y;
        BOOST_CHECK((x & y) == t);
        t = x;
        t |= y;
        BOOST_CHECK((x | y) == t);
        t = x;
        t ^= y;
        BOOST_CHECK((x ^ y) == t);
        t = x;
        t.flip();
        BOOST_CHECK((~x) == t);
        BOOST_CHECK(x[1] and x[2]); // and none of them wrote through
        BOOST_CHECK((~~x) == x);
        BOOST_CHECK(((x & y) | (x ^ y)) == (x | y)); // one identity, over packed bits

        auto c = Storage();
        auto d = Storage();
        d.set(1);
        View const v(c);
        v |= View(d);
        BOOST_CHECK(c.test(1) and not c.test(2));
        swap(x, y);
        BOOST_CHECK(x[3] and y[1]);
}

// The ordering invariant on the trait's entry, the only ordering an owner has.
BOOST_AUTO_TEST_CASE(TheOrderingIsTheLexicographicOrderOfTheBools)
{
        using Packed = xstd::basic_bit_array<std::uint8_t, 9>;
        static_assert(std::regular<Packed> and std::totally_ordered<Packed>);

        auto const patterns = std::vector<std::vector<std::size_t>>{{}, {0}, {1}, {0, 1}, {8}, {0, 8}};
        for (auto const& p : patterns) {
                for (auto const& q : patterns) {
                        auto x = Packed();
                        auto y = Packed();
                        for (auto const i : p) {
                                x[i] = true;
                        }
                        for (auto const i : q) {
                                y[i] = true;
                        }

                        auto const expected = std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());
                        BOOST_CHECK((x <=> y) == expected);
                        BOOST_CHECK((x == y) == (p == q));
                }
        }
}

// Two sizes compare as the bools do: the shared positions decide, and agreeing, the shorter is less.
BOOST_AUTO_TEST_CASE(TheOrderingAcrossTwoSizesIsStillTheLexicographicOrder)
{
        auto const cases = dynamic_probes();
        for (auto const& [x, vx] : cases) {
                for (auto const& [y, vy] : cases) {
                        auto const expected = std::lexicographical_compare_three_way(vx.begin(), vx.end(), vy.begin(), vy.end());
                        BOOST_CHECK((x <=> y) == expected);
                        BOOST_CHECK((y <=> x) == (0 <=> expected));
                        BOOST_CHECK((x == y) == (vx == vy));
                }
        }
}

// Dependent, so a constrained-away member is a false rather than a hard error.
template<class X>
constexpr bool can_grow = requires (X& x) { x.push_back(true); x.pop_back(); x.resize(1UZ); x.resize(1UZ, true); x.clear(); x.reserve(1UZ); x.shrink_to_fit(); };

// Growth is the owner's over storage that grows; a static width and a view have none of it.
BOOST_AUTO_TEST_CASE(GrowthIsTheOwnersOverStorageThatGrows)
{
        using Dynamic = xstd::bits::detail::sequence_adaptor<xstd::bits::detail::contiguous_bit_container<std::vector<std::uint64_t>>, xstd::bits::detail::storage::owned, xstd::bits::detail::window::all>;
        using Span = xstd::bits::detail::sequence_adaptor<xstd::bits::detail::contiguous_bit_container<std::vector<std::uint64_t>>, xstd::bits::detail::storage::borrowed, xstd::bits::detail::window::all>;

        static_assert(can_grow<Dynamic>);
        static_assert(not can_grow<Owner>);
        static_assert(not can_grow<Span>);

        auto d = Dynamic(3, true);
        d.push_back(false);
        BOOST_CHECK_EQUAL(d.size(), 4UZ);
        BOOST_CHECK(std::ranges::equal(d, std::vector<bool>{true, true, true, false}));
        // What a distance can name: a random access range counts its positions by a difference_type.
        BOOST_CHECK_EQUAL(d.max_size(), xstd::bits::detail::contiguous_bit_container<std::vector<std::uint64_t>>::max_addressable_width);
        BOOST_CHECK_LT(d.max_size(), xstd::bits::detail::contiguous_bit_container<std::vector<std::uint64_t>>().max_size());
        BOOST_CHECK_THROW(d.resize(d.max_size() + 1UZ), std::length_error);
        BOOST_CHECK_EQUAL(Owner().max_size(), 100UZ);

        // The fill insert asks for size() + n, which saturates rather than wrapping to a shorter sequence.
        BOOST_CHECK_THROW(d.insert(d.begin(), std::numeric_limits<std::size_t>::max(), true), std::length_error);
        BOOST_CHECK_EQUAL(d.size(), 4UZ);
        BOOST_CHECK(std::ranges::equal(d, std::vector<bool>{true, true, true, false}));

        // The one a width can hold is unaffected, and lands where it was asked for.
        d.insert(d.begin(), 2UZ, false);
        BOOST_CHECK(std::ranges::equal(d, std::vector<bool>{false, false, true, true, true, false}));
}

// at() is the reading's one checked door, measured against the width that grows.
BOOST_AUTO_TEST_CASE(AtAnswersAtARunTimeWidthToo)
{
        auto d = xstd::bits::detail::sequence_adaptor<xstd::bits::detail::contiguous_bit_container<std::vector<std::uint64_t>>, xstd::bits::detail::storage::owned, xstd::bits::detail::window::all>(3, true);
        BOOST_CHECK_THROW(static_cast<void>(d.at(3UZ)), std::out_of_range);
        d.push_back(false);
        BOOST_CHECK(d.at(3UZ) == false);
        BOOST_CHECK(d.at(0UZ) == true);
}

namespace {

using Dynamic = xstd::bits::detail::sequence_adaptor<xstd::bits::detail::contiguous_bit_container<std::vector<std::uint64_t>>, xstd::bits::detail::storage::owned, xstd::bits::detail::window::all>;

// One functor at namespace scope, so the packing tier is instantiated once rather than once per closure.
constexpr auto every_third = [](std::size_t i) -> bool { return i % 3 == 0; };

// The same functor over a bound the sequence can hold, and over one it cannot.
[[nodiscard]] auto counting_to(std::size_t n)
{
        return std::views::iota(0UZ, n) | std::views::transform(every_third);
}

} // namespace

// The packing tier over a computed range; two lengths, 128 ending on a word boundary and 70 not.
BOOST_AUTO_TEST_CASE(TheAppendsPackWhatTheRangeComputes)
{
        auto d = Dynamic();
        d.append_range(counting_to(128UZ));
        BOOST_CHECK_EQUAL(d.size(), 128UZ);

        d.append_range(counting_to(70UZ));
        BOOST_CHECK_EQUAL(d.size(), 198UZ);

        auto expected = std::vector<bool>();
        for (auto const n : {128UZ, 70UZ}) {
                for (auto const i : std::views::iota(0UZ, n)) {
                        expected.push_back(every_third(i));
                }
        }
        BOOST_CHECK(std::ranges::equal(d, expected));
}

// The other addition a caller names without holding it: a sized range's size() over elements it never makes.
BOOST_AUTO_TEST_CASE(TheAppendsSumSaturatesRatherThanWrapping)
{
        constexpr auto top = std::numeric_limits<std::size_t>::max();

        auto d = Dynamic();
        BOOST_CHECK_THROW(d.append_range(counting_to(top)), std::length_error);
        BOOST_CHECK(d.empty());

        d.push_back(true);
        BOOST_CHECK_THROW(d.append_range(counting_to(top)), std::length_error);
        BOOST_CHECK_EQUAL(d.size(), 1UZ);
}

// The same sum through insert_range: the copy is what the length_error unwinds, so the sequence is untouched.
BOOST_AUTO_TEST_CASE(ARefusedInsertRangeLeavesTheSequenceAsItWas)
{
        constexpr auto top = std::numeric_limits<std::size_t>::max();

        auto d = Dynamic(3, true);
        d.push_back(false);
        BOOST_CHECK_THROW(d.insert_range(d.cend(), counting_to(top)), std::length_error);
        BOOST_CHECK(std::ranges::equal(d, std::vector<bool>{true, true, true, false}));
}

BOOST_AUTO_TEST_CASE(AZeroWidthSequenceIsEmpty)
{
        auto const a = xstd::basic_bit_array<std::uint8_t, 0>();
        BOOST_CHECK(a.empty() and a.begin() == a.end());
        auto c = xstd::bits::detail::contiguous_bit_container<std::array<std::uint8_t, 1>, 0>();
        auto const v = xstd::bits::detail::sequence_adaptor<xstd::bits::detail::contiguous_bit_container<std::array<std::uint8_t, 1>, 0>, xstd::bits::detail::storage::borrowed, xstd::bits::detail::window::all>(c);
        BOOST_CHECK(v.empty() and v.begin() == v.end());
}

// The sequence reading's own aggregates, against the reading they belong to.
namespace {

using Graded = test::graded_extents<xstd::basic_bit_array>;

// One bit of pattern p at position i, as bit_array's model cases have it.
auto pattern_bit(std::size_t p, std::size_t i, std::size_t n)
        -> bool
{
        switch (p) {
                case 0UZ:
                        return false;
                case 1UZ:
                        return true;
                case 2UZ:
                        return i == 0UZ;
                case 3UZ:
                        return i + 1UZ == n;
                case 4UZ:
                        return (i % 2UZ) == 0UZ;
                default:
                        return (i % 3UZ) == 0UZ;
        }
}

// The model at the same extent, written through the sequence under test so the two are filled by one loop.
template<class Seq>
auto write_pattern(Seq& s, std::size_t p)
        -> std::vector<bool>
{
        auto m = std::vector<bool>(s.size());
        for (auto const i : std::views::iota(0UZ, s.size())) {
                bool const bit = pattern_bit(p, i, s.size());
                s[i] = bit;
                m[i] = bit;
        }
        return m;
}

// Counted rather than asserted per position, so a failure names the operation instead of drowning the log.
template<class Seq>
auto aggregate_disagreements(Seq const& s, std::vector<bool> const& m)
        -> std::size_t
{
        auto disagreements = 0UZ;
        for (auto const value : {true, false}) {
                auto const is = [value](bool b) -> bool { return b == value; };
                disagreements += static_cast<std::size_t>(s.count(value) != static_cast<std::size_t>(std::ranges::count(m, value)));
                // And against the generic algorithm over the sequence itself, which the benchmark's ratio assumes.
                disagreements += static_cast<std::size_t>(s.count(value) != static_cast<std::size_t>(std::ranges::count(s, value)));
                disagreements += static_cast<std::size_t>(s.all(value) != std::ranges::all_of(m, is));
                disagreements += static_cast<std::size_t>(s.any(value) != std::ranges::any_of(m, is));
                disagreements += static_cast<std::size_t>(s.none(value) != std::ranges::none_of(m, is));
        }
        // The argument defaults to the true arm, which is where the bitset reading's four already are.
        disagreements += static_cast<std::size_t>(s.count() != s.count(true));
        disagreements += static_cast<std::size_t>(s.all() != s.all(true));
        disagreements += static_cast<std::size_t>(s.any() != s.any(true));
        disagreements += static_cast<std::size_t>(s.none() != s.none(true));
        // The four identities the false arms are, spelled here because they are the implementation.
        disagreements += static_cast<std::size_t>(s.count(false) != s.size() - s.count(true));
        disagreements += static_cast<std::size_t>(s.all(false) != s.none(true));
        disagreements += static_cast<std::size_t>(s.any(false) != not s.all(true));
        disagreements += static_cast<std::size_t>(s.none(false) != s.all(true));
        return disagreements;
}

// What for_each hands its functor, in the order it hands it: the range-for's own answer, which is the contract.
template<class Seq>
auto for_each_bools(Seq const& s)
        -> std::vector<bool>
{
        auto v = std::vector<bool>();
        s.for_each([&v](bool b) -> void { v.push_back(b); });
        return v;
}

} // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(TheAggregatesAgreeWithTheModel, T, Graded)
{
        auto disagreements = 0UZ;
        for (auto const p : std::views::iota(0UZ, 6UZ)) {
                auto a = T();
                auto const m = write_pattern(a, p);
                disagreements += aggregate_disagreements(a, m);
        }
        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

// The same over a window: a masked word at a time, at every offset and length, so both ends are exercised.
BOOST_AUTO_TEST_CASE(TheAggregatesAgreeWithTheModelOnAWindowOfOurs)
{
        using Storage24 = xstd::bits::detail::contiguous_bit_container<std::array<std::uint8_t, 3>, 24>;
        auto disagreements = 0UZ;
        for (auto const p : std::views::iota(0UZ, 6UZ)) {
                auto c = Storage24();
                auto v = xstd::bit_span(c);
                auto const m = write_pattern(v, p);
                for (auto const off : std::views::iota(0UZ, v.size() + 1UZ)) {
                        for (auto const count : std::views::iota(0UZ, v.size() - off + 1UZ)) {
                                auto const w = v.subspan(off, count);
                                auto const mw = std::vector<bool>(m.begin() + static_cast<std::ptrdiff_t>(off), m.begin() + static_cast<std::ptrdiff_t>(off + count));
                                disagreements += aggregate_disagreements(w, mw);
                                disagreements += static_cast<std::size_t>(not std::ranges::equal(for_each_bools(w), mw));
                        }
                }
        }
        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

// std::mismatch's answer over the machinery operator== is made of: the position, or size() where equal.
BOOST_AUTO_TEST_CASE_TEMPLATE(MismatchAgreesWithTheModel, T, Graded)
{
        auto disagreements = 0UZ;
        for (auto const p : std::views::iota(0UZ, 6UZ)) {
                for (auto const q : std::views::iota(0UZ, 6UZ)) {
                        auto x = T();
                        auto y = T();
                        auto const mx = write_pattern(x, p);
                        auto const my = write_pattern(y, q);
                        auto const [i, j] = std::ranges::mismatch(mx, my);
                        auto const expected = static_cast<std::size_t>(i - mx.begin());
                        disagreements += static_cast<std::size_t>(x.mismatch(y) != expected);
                        // Symmetric, and equal values answer the width rather than any position in it.
                        disagreements += static_cast<std::size_t>(y.mismatch(x) != expected);
                        disagreements += static_cast<std::size_t>(x.mismatch(x) != x.size());
                }
        }
        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

// Dependent, so a constrained-away member is a false rather than a hard error.
template<class S>
constexpr bool can_mismatch = requires (S const& a) { a.mismatch(a); };

// What for_each accepts, likewise dependent.
template<class S, class F>
constexpr bool walks = requires (S const& s, F f) { s.for_each(f); };

// Functors overloaded on the value category.
struct void_probe
{
        bool& took_a_reference;

        auto operator()(bool&&) const -> void {}

        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(bool&) const
                -> void
        {
                took_a_reference = true;
        }
};

struct bool_probe
{
        bool& took_a_reference;

        auto operator()(bool&&) const
                -> bool
        {
                return true;
        }

        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(bool&) const
                -> bool
        {
                took_a_reference = true;
                return true;
        }
};

// A window's blocks are not its own, so it has no mismatch; nor has an owner over storage without the entry.
BOOST_AUTO_TEST_CASE(MismatchIsTheOwnersOverStorageThatHasTheEntry)
{
        static_assert(can_mismatch<Owner>);
        static_assert(can_mismatch<View>);
        static_assert(not can_mismatch<View::subspan_type>);
}

// for_each hands the functor what the iterator dereferences to, and stops where a bool functor says to.
BOOST_AUTO_TEST_CASE_TEMPLATE(ForEachAgreesWithTheRangeFor, T, Graded)
{
        auto disagreements = 0UZ;
        for (auto const p : std::views::iota(0UZ, 6UZ)) {
                auto a = T();
                auto const m = write_pattern(a, p);
                disagreements += static_cast<std::size_t>(not std::ranges::equal(for_each_bools(a), m));

                // A void functor always continues; a bool one says, and three is inside all but two extents.
                auto seen = 0UZ;
                a.for_each([&seen](bool) -> bool { return ++seen < 3UZ; });
                disagreements += static_cast<std::size_t>(seen != std::ranges::min(a.size(), 3UZ));
        }
        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

// The functor is handed the bool by value, and the constraint says so.
BOOST_AUTO_TEST_CASE(ForEachHandsTheBoolByValue)
{
        // By value, generic or not, and by const reference: all four read what they are given.
        static_assert(walks<Owner, decltype([](bool) -> void {})>);
        static_assert(walks<Owner, decltype([](auto) -> void {})>);
        static_assert(walks<Owner, decltype([](bool const&) -> void {})>);
        static_assert(walks<Owner, decltype([](auto const&) -> void {})>);

        // A plain function is a functor too, and stays one.
        static_assert(walks<Owner, void (*)(bool)>);

        // A functor returning bool to mean "keep going" is the other accepted shape.
        static_assert(walks<Owner, decltype([](bool) -> bool { return true; })>);

        // And the two that would have written to nothing.
        static_assert(not walks<Owner, decltype([](bool&) -> void {})>);
        static_assert(not walks<Owner, decltype([](auto&) -> void {})>);

        // Every shape is constrained alike: a view and a window.
        static_assert(not walks<View, decltype([](bool&) -> void {})>);
        static_assert(not walks<View::subspan_type, decltype([](bool&) -> void {})>);

        // Writing through the sequence is the range-for's job, and it still is.
        auto a = Owner();
        // const, and it still writes: assigning through the proxy is what the proxy is for.
        for (auto const r : a) {
                r = true;
        }
        BOOST_CHECK_EQUAL(a.count(), a.size());

        // And the overload resolution the constraint cannot reach: an lvalue at the call would take the reference.
        auto took_a_reference = false;
        a.for_each(void_probe{took_a_reference});
        BOOST_CHECK(not took_a_reference);
        a.for_each(bool_probe{took_a_reference});
        BOOST_CHECK(not took_a_reference);
}

// The byte exchange at the sequence reading: byte j holds [8j, 8j + 8), least significant bit first.
BOOST_AUTO_TEST_CASE(APackedArrayExchangesBytesWithAFieldOfBits)
{
        constexpr auto N = 100UZ;
        auto src = std::bitset<N>();
        for (auto i = 0UZ; i < N; i += 7UZ) {
                src.set(i);
        }

        auto const a = xstd::bit_cast<xstd::bit_array<N>>(src);
        BOOST_CHECK_EQUAL(a.count(), src.count());
        for (auto const i : std::views::iota(0UZ, N)) {
                BOOST_CHECK_EQUAL(a[i], src.test(i));
        }
        BOOST_CHECK(a.to_bits<std::bitset<N>>() == src);

        // At compile time too, and at a width that is a whole number of bytes and one that is not.
        static_assert([] -> bool {
                auto const bs = std::bitset<64>(0xDEAD'BEEF'0123'4567ULL);
                return xstd::bit_cast<xstd::bit_array<64>>(bs).to_bits<std::bitset<64>>() == bs;
        }());
        static_assert([] -> bool {
                auto const bs = std::bitset<17>(0x1'5A5AULL);
                return xstd::bit_cast<xstd::bit_array<17>>(bs).to_bits<std::bitset<17>>() == bs;
        }());
}

// Named in both directions: a reader picks the reading rather than letting a conversion pick for them.
BOOST_AUTO_TEST_CASE(TheSequenceExchangeIsNamedBothWays)
{
        constexpr auto N = 64UZ;
        using T = xstd::bit_array<N>;
        static_assert(test::casts_between<T, std::bitset<N>>);

        // The unnamed doors are closed, in both directions.
        static_assert(not std::is_constructible_v<T, std::bitset<N>>);
        static_assert(not std::is_constructible_v<std::bitset<N>, T>);

        // Any other width is no exchange at all, rather than a narrowing one.
        static_assert(not test::casts_from<T, std::bitset<N + 1UZ>>);
        static_assert(not test::casts_from<T, std::bitset<N - 1UZ>>);
}

// A window is the one shape that must not convert: its position zero is not the storage's.
BOOST_AUTO_TEST_CASE(AWindowIsNotAFieldOfBitsButAPlainViewIs)
{
        static_assert(not test::exchanges_to_bits<xstd::bit_subspan<std::array<std::uint64_t, 2>, std::dynamic_extent, 100>, std::bitset<100>>);
        static_assert(test::exchanges_to_bits<View, std::bitset<100>>);
        static_assert(test::exchanges_to_bits<Reader, std::bitset<100>>);

        // A view reads the bits it spans, which are the whole container's.
        auto storage = Storage();
        // const, because a view's const is SHALLOW: the handle does not change, the bits it refers to do.
        auto const view = View(storage);
        view[0] = true;
        view[Storage::extent - 1UZ] = true;
        auto const out = view.to_bits<std::bitset<100>>();
        BOOST_CHECK_EQUAL(out.count(), 2UZ);
        BOOST_CHECK(out.test(0) and out.test(Storage::extent - 1UZ));

        // A view never gains the INBOUND direction, which would write through bits it does not own.
        static_assert(not test::casts_from<View, std::bitset<100>>);
}

// A run-time width has neither: a field of N bits names one N, and a growing sequence has no single one.
BOOST_AUTO_TEST_CASE(ARunTimeWidthHasNoByteExchange)
{
        static_assert(not test::casts_from<xstd::bit_vector, std::bitset<64>>);
        static_assert(not test::exchanges_to_bits<xstd::bit_vector, std::bitset<64>>);
        static_assert(not test::casts_from<DynamicOctet, std::bitset<64>>);
}

// Raw blocks read differently here than at the set reading: five is true, false, true, then false.
BOOST_AUTO_TEST_CASE(RawBlocksAreElementsUnderThisReading)
{
        auto const a = xstd::bit_array<64>(xstd::from_bit_storage, std::array<std::uint64_t, 1>{5ULL});
        // Combined with `and`: a bare proxy is an ambiguous initializer for Boost.Test's assertion_result.
        BOOST_CHECK(a[0] and not a[1] and a[2]);
        BOOST_CHECK_EQUAL(a.count(), 2UZ);

        static_assert([] -> bool {
                auto const b = std::array<std::uint64_t, 2>{0xF0F0ULL, 3ULL};
                return xstd::bit_array<128>(xstd::from_bit_storage, b).to_bits<std::array<std::uint64_t, 2>>() == b;
        }());
}

BOOST_AUTO_TEST_SUITE_END()
