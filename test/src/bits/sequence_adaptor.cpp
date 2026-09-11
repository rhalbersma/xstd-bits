//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>              // graded_extents
#include <xstd/bits/bit_array.hpp>           // bit_array
#include <xstd/bits/bit_span.hpp>            // bit_span
#include <xstd/bits/bit_traits.hpp>          // bit_traits, block_readable
#include <xstd/bits/detail/block_array.hpp>  // block_array
#include <xstd/bits/detail/block_vector.hpp> // block_vector
#include <xstd/bits/ext/std/bitset.hpp>      // bit_traits over std::bitset
#include <xstd/bits/ownership.hpp>           // ownership
#include <xstd/bits/sequence_adaptor.hpp>    // sequence_adaptor
#include <boost/test/unit_test.hpp>          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                         // all_of, any_of, count, equal, lexicographical_compare_three_way, mismatch, none_of
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

using Storage = xstd::detail::bits::block_array<std::uint64_t, 100>;
using Owner   = xstd::basic_bit_array<std::uint64_t, 100>;
using View    = xstd::sequence_adaptor<Storage, xstd::ownership::refers, false>;
using Reader  = xstd::sequence_adaptor<Storage const, xstd::ownership::refers, false>;

// Dependent, so an absent member is a false rather than a hard error.
template<class S> constexpr bool can_fill  = requires (S s) { s.fill(true); };
template<class S> constexpr bool can_write = requires (S s) { s[0] = true; };
template<class S> constexpr bool can_swap  = requires (S s) { s.swap(s); };

template<class Seq>
[[nodiscard]] auto bools(Seq const& s)
        -> std::vector<bool>
{
        return { s.begin(), s.end() };
}

// A storage that keeps its blocks to itself on every standard library, which a std::bitset is not: libc++ has no
// block entry at all, libstdc++ none above the portable to_ullong() read, and MSVC one at every width through
// _Getword. So the position tier gets a fixture of its own, the required entries and a write and nothing more,
// as test/src/bits/bit_traits.cpp's element_bits is. [design.md#detection-by-absence]
template<std::size_t N>
struct element_bits
{
        xstd::detail::bits::block_array<std::uint8_t, N> bits{};
};

}       // namespace

namespace xstd {

template<std::size_t N>
struct bit_traits<element_bits<N>>
{
        using bits_type = element_bits<N>;

        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size(bits_type const&)                  noexcept -> std::size_t { return N; }
        [[nodiscard]] static constexpr auto at  (bits_type const& c, std::size_t n) noexcept -> bool        { return c.bits.test(n); }

        static constexpr auto unchecked_assign(bits_type& c, std::size_t n, bool value) noexcept
                -> void
        {
                if (value) {
                        c.bits.set(n);
                } else {
                        c.bits.reset(n);
                }
        }
};

}       // namespace xstd

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
        using Packed = xstd::basic_bit_array<std::uint8_t, 9>;
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
        using Dynamic = xstd::sequence_adaptor<xstd::detail::bits::block_vector<std::uint64_t>, xstd::ownership::owns, false>;
        using Span    = xstd::sequence_adaptor<xstd::detail::bits::block_vector<std::uint64_t>, xstd::ownership::refers, false>;

        static_assert(    can_grow<Dynamic>);
        static_assert(not can_grow<Owner>);
        static_assert(not can_grow<Span>);

        auto d = Dynamic(3, true);
        d.push_back(false);
        BOOST_CHECK_EQUAL(d.size(), 4UZ);
        BOOST_CHECK(std::ranges::equal(d, std::vector<bool>{ true, true, true, false }));
        BOOST_CHECK_EQUAL(d.max_size(), xstd::detail::bits::block_vector<std::uint64_t>().max_size());
        BOOST_CHECK_LT(d.max_size(), std::numeric_limits<std::size_t>::max());
        BOOST_CHECK_EQUAL(Owner().max_size(), 100UZ);
}

BOOST_AUTO_TEST_CASE(AZeroWidthSequenceIsEmpty)
{
        auto const a = xstd::basic_bit_array<std::uint8_t, 0>();
        BOOST_CHECK(a.empty() and a.begin() == a.end());
        auto c = xstd::detail::bits::block_array<std::uint8_t, 0>();
        auto const v = xstd::sequence_adaptor<xstd::detail::bits::block_array<std::uint8_t, 0>, xstd::ownership::refers, false>(c);
        BOOST_CHECK(v.empty() and v.begin() == v.end());
}

// The sequence reading's own aggregates, against the reading they belong to rather than the set reading that
// happens to answer the same integer for one of the eight. Every operation on the packing and on the
// std::vector<bool> it is held against, at every graded extent and block type so a block boundary lands
// mid-pattern, and at both values of the bool. [design.md#the-sequence-aggregates]
namespace {

using Graded = test::graded_extents<xstd::basic_bit_array>;

// One bit of pattern p at position i, as bit_array's model cases have it. Empty and full are the two degenerate
// widths the aggregates disagree about most: they are the fixed points of all and none.
auto pattern_bit(std::size_t p, std::size_t i, std::size_t n)
        -> bool
{
        switch (p) {
        case 0UZ: return false;
        case 1UZ: return true;
        case 2UZ: return i == 0UZ;
        case 3UZ: return i + 1UZ == n;
        case 4UZ: return (i % 2UZ) == 0UZ;
        default:  return (i % 3UZ) == 0UZ;
        }
}

// The model at the same extent, written through the sequence under test so the two are filled by one loop.
template<class Seq>
auto write_pattern(Seq& s, std::size_t p)
        -> std::vector<bool>
{
        auto m = std::vector<bool>(s.size());
        for (auto i = 0UZ; i < s.size(); ++i) {
                bool const bit = pattern_bit(p, i, s.size());
                s[i] = bit;
                m[i] = bit;
        }
        return m;
}

// Counted rather than asserted per position, so a failure names the operation instead of drowning the log.
// [design.md#counted-not-asserted]
template<class Seq>
auto aggregate_disagreements(Seq const& s, std::vector<bool> const& m)
        -> std::size_t
{
        auto disagreements = 0UZ;
        for (auto const value : { true, false }) {
                auto const is = [value](bool b) -> bool { return b == value; };
                disagreements += static_cast<std::size_t>(s.count(value) != static_cast<std::size_t>(std::ranges::count(m, value)));
                disagreements += static_cast<std::size_t>(s.all (value) != std::ranges::all_of (m, is));
                disagreements += static_cast<std::size_t>(s.any (value) != std::ranges::any_of (m, is));
                disagreements += static_cast<std::size_t>(s.none(value) != std::ranges::none_of(m, is));
        }
        // The argument defaults to the true arm, which is where the bitset reading's four already are.
        disagreements += static_cast<std::size_t>(s.count() != s.count(true));
        disagreements += static_cast<std::size_t>(s.all()   != s.all(true));
        disagreements += static_cast<std::size_t>(s.any()   != s.any(true));
        disagreements += static_cast<std::size_t>(s.none()  != s.none(true));
        // The four identities the false arms are, spelled here because they are the implementation.
        disagreements += static_cast<std::size_t>(s.count(false) != s.size() - s.count(true));
        disagreements += static_cast<std::size_t>(s.all(false)   != s.none(true));
        disagreements += static_cast<std::size_t>(s.any(false)   != not s.all(true));
        disagreements += static_cast<std::size_t>(s.none(false)  != s.all(true));
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

}       // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(TheAggregatesAgreeWithTheModel, T, Graded)
{
        auto disagreements = 0UZ;
        for (auto p = 0UZ; p < 6UZ; ++p) {
                auto a = T();
                auto const m = write_pattern(a, p);
                disagreements += aggregate_disagreements(a, m);
        }
        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

// The same over a window, whose blocks are not its own: a masked word at a time, at every offset and every
// length, so the mask is exercised at both ends of a word rather than only at the top. [design.md#windows]
BOOST_AUTO_TEST_CASE(TheAggregatesAgreeWithTheModelOnAWindowOfOurs)
{
        using Storage24 = xstd::detail::bits::block_array<std::uint8_t, 24>;
        auto disagreements = 0UZ;
        for (auto p = 0UZ; p < 6UZ; ++p) {
                auto c = Storage24();
                auto v = xstd::bit_span(c);
                auto const m = write_pattern(v, p);
                for (auto off = 0UZ; off <= v.size(); ++off) {
                        for (auto count = 0UZ; off + count <= v.size(); ++count) {
                                auto const w = v.subspan(off, count);
                                auto const mw = std::vector<bool>(m.begin() + static_cast<std::ptrdiff_t>(off), m.begin() + static_cast<std::ptrdiff_t>(off + count));
                                disagreements += aggregate_disagreements(w, mw);
                                disagreements += static_cast<std::size_t>(not std::ranges::equal(for_each_bools(w), mw));
                        }
                }
        }
        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

// And over a window of a storage that keeps its blocks to itself, which is the one position-at-a-time tier.
// [design.md#detection-by-absence]
BOOST_AUTO_TEST_CASE(TheAggregatesAgreeWithTheModelOnAWindowOfAnythingElse)
{
        using Wide = element_bits<100>;
        static_assert(not xstd::block_readable<xstd::bit_traits<Wide>, Wide>);

        auto disagreements = 0UZ;
        for (auto p = 0UZ; p < 6UZ; ++p) {
                auto c = Wide();
                auto v = xstd::bit_span(c);
                auto const m = write_pattern(v, p);
                for (auto const off : { 0UZ, 1UZ, 7UZ, 63UZ, 100UZ }) {
                        for (auto const count : { 0UZ, 1UZ, 9UZ, 37UZ }) {
                                if (off + count > v.size()) {
                                        continue;
                                }
                                auto const w = v.subspan(off, count);
                                auto const mw = std::vector<bool>(m.begin() + static_cast<std::ptrdiff_t>(off), m.begin() + static_cast<std::ptrdiff_t>(off + count));
                                disagreements += aggregate_disagreements(w, mw);
                                disagreements += static_cast<std::size_t>(not std::ranges::equal(for_each_bools(w), mw));

                                // The position tier's own early exit, which the block tier's case below covers for it.
                                auto seen = 0UZ;
                                w.for_each([&seen](bool) -> bool { return ++seen < 3UZ; });
                                disagreements += static_cast<std::size_t>(seen != std::ranges::min(w.size(), 3UZ));
                        }
                }
        }
        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

// std::mismatch's answer, over the machinery operator== is already made of: the position, or size() where the two
// agree. Every pair of patterns, so the answer lands inside a block, on a boundary and past the end.
BOOST_AUTO_TEST_CASE_TEMPLATE(MismatchAgreesWithTheModel, T, Graded)
{
        auto disagreements = 0UZ;
        for (auto p = 0UZ; p < 6UZ; ++p) {
                for (auto q = 0UZ; q < 6UZ; ++q) {
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
template<class S> constexpr bool can_mismatch = requires (S const& a) { a.mismatch(a); };

// What for_each accepts, likewise dependent.
template<class S, class F> constexpr bool walks = requires (S const& s, F f) { s.for_each(f); };

// Functors overloaded on the value category. The constraint above cannot tell the two overloads apart -- both
// make the functor invocable with a bool -- so these, and only these, are what pin the prvalue at the call:
// an lvalue at the call takes the && overload away and lands on the & one. One probe per arm, because
// invoke_continues splits on the return type and each arm calls the functor for itself.
struct void_probe
{
        bool& took_a_reference;

        auto operator()(bool&&) const -> void {}
        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(bool&) const -> void { took_a_reference = true; }
};

struct bool_probe
{
        bool& took_a_reference;

        auto operator()(bool&&) const -> bool { return true; }
        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(bool&) const -> bool { took_a_reference = true; return true; }
};

// A window's blocks are not its own, so it has no mismatch; nor has an owner over storage without the entry.
BOOST_AUTO_TEST_CASE(MismatchIsTheOwnersOverStorageThatHasTheEntry)
{
        static_assert(can_mismatch<Owner>);
        static_assert(can_mismatch<View>);
        static_assert(not can_mismatch<View::subspan_type>);
        static_assert(not can_mismatch<xstd::sequence_adaptor<std::bitset<9>, xstd::ownership::owns, false>>);
}

// for_each hands the functor what the iterator dereferences to, in the same order, and stops where a bool functor
// says to: the range-for's answer by a loop structure no iterator can express. [design.md#the-sequence-for-each]
BOOST_AUTO_TEST_CASE_TEMPLATE(ForEachAgreesWithTheRangeFor, T, Graded)
{
        auto disagreements = 0UZ;
        for (auto p = 0UZ; p < 6UZ; ++p) {
                auto a = T();
                auto const m = write_pattern(a, p);
                disagreements += static_cast<std::size_t>(not std::ranges::equal(for_each_bools(a), m));

                // A void functor always continues; a bool one says, and three is inside every extent but the two smallest.
                auto seen = 0UZ;
                a.for_each([&seen](bool) -> bool { return ++seen < 3UZ; });
                disagreements += static_cast<std::size_t>(seen != std::ranges::min(a.size(), 3UZ));
        }
        BOOST_CHECK_EQUAL(disagreements, 0UZ);
}

// The functor is handed the bool by value, and the constraint says so. Without that a functor asking for bool&
// binds to the walker's own local: the walk reads the storage through a const reference and never writes back, so
// what looks like a mutating pass compiles into a silent no-op. Writing through the sequence is what the range-for
// and its proxy are for. [design.md#the-functor-takes-a-value]
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

        // And the two that would have written to nothing. auto is no help here: it is the reference that is the
        // fault, not the spelling of the type, and [](auto&) is exactly as silent as [](bool&) was.
        static_assert(not walks<Owner, decltype([](bool&) -> void {})>);
        static_assert(not walks<Owner, decltype([](auto&) -> void {})>);

        // Every shape is constrained alike: a view, a window, and a storage that keeps its blocks to itself.
        static_assert(not walks<View, decltype([](bool&) -> void {})>);
        static_assert(not walks<View::subspan_type, decltype([](bool&) -> void {})>);
        static_assert(not walks<xstd::sequence_adaptor<element_bits<100>, xstd::ownership::refers, false>, decltype([](bool&) -> void {})>);

        // Writing through the sequence is the range-for's job, and it still is.
        auto a = Owner();
        // const, and it still writes: assigning through the proxy is what the proxy is for. [design.md#proxies-compare-themselves]
        for (auto const r : a) { r = true; }
        BOOST_CHECK_EQUAL(a.count(), a.size());

        // And the overload resolution the constraint cannot reach: an lvalue at the call would take the reference.
        // Both arms, the void one and the bool one, each of which calls the functor itself.
        auto took_a_reference = false;
        a.for_each(void_probe{ took_a_reference });
        BOOST_CHECK(not took_a_reference);
        a.for_each(bool_probe{ took_a_reference });
        BOOST_CHECK(not took_a_reference);
}

BOOST_AUTO_TEST_SUITE_END()
