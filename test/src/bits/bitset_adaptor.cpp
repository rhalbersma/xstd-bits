//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bit_exchange.hpp>                         // exchanges_bits, exchanges_from_bits, exchanges_to_bits
#include <xstd/bits/bit_set_view.hpp>                    // bit_set_view
#include <xstd/bits/bit_span.hpp>                        // bit_span
#include <xstd/bits/contiguous_bit_sequence.hpp>          // contiguous_bit_sequence
#include <xstd/bits/bitset.hpp>                          // basic_bitset, bitset
#include <xstd/bits/bitset_adaptor.hpp>                  // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_array.hpp>     // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_vector.hpp>    // contiguous_bit_vector
#include <xstd/bits/dynamic_bitset.hpp>                  // basic_dynamic_bitset
#include <boost/dynamic_bitset.hpp>                      // dynamic_bitset
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                     // equal
#include <array>                                         // array
#include <bitset>                                        // bitset
#include <compare>                                       // is_lt, strong_ordering
#include <concepts>                                      // regular, same_as, totally_ordered
#include <cstddef>                                       // size_t
#include <cwchar>                                        // mbstate_t
#include <cstdint>                                       // uint8_t, uint64_t
#include <functional>                                    // hash
#include <ios>                                           // streamoff
#include <iosfwd>                                        // streampos
#include <iterator>                                      // back_inserter, contiguous_iterator
#include <limits>                                        // numeric_limits
#include <list>                                          // list
#include <ranges>                                        // equal, iota, range, reverse
#include <sstream>                                       // istringstream
#include <stdexcept>                                     // invalid_argument, out_of_range, overflow_error
#include <string>                                        // char_traits, string
#include <string_view>                                   // basic_string_view
#include <tuple>                                         // tuple
#include <type_traits>                                   // is_constructible_v, is_convertible_v, is_nothrow_*, is_trivially_*
#include <utility>                                       // as_const, declval
#include <vector>                                        // vector

// A program-defined char-like type, to hold the const charT* constructor to LWG 4294's four traits rather than to a
// list of the five character types the standard happens to specialize char_traits for. std::bitset takes this; so,
// now, does the wrapper. Outside the suite, because the specialization below has to be at namespace scope.
//
// A NAMED namespace, so the type and its traits have external linkage. In an anonymous one clang reports every
// char_traits member this file never calls -- which is most of them, a char_traits existing to be called by
// basic_string_view rather than by us -- under -Wunused-member-function and -Wunneeded-member-function, and this
// tree compiles with -Werror. External linkage is the shape a type a std:: template is specialized on wants anyway.
namespace test_chars {

struct digit_char
{
        // NO default member initializer: one would make this non-trivially-default-constructible, which is the third
        // of LWG 4294's four traits and the one libstdc++ asserts by name inside basic_string and basic_string_view.
        unsigned char v;

        // Defaulted on first declaration, so the default constructor below leaves it trivial and all four traits hold.
        constexpr digit_char() noexcept = default;

        // And a converting constructor, because the string constructors spell their defaults charT('0') -- as
        // [bitset.cons] spells them. On a bare aggregate that is parenthesized aggregate initialization, which clang
        // diagnoses as a C++20 extension and -Werror turns into an error; libstdc++'s own bitset does exactly the
        // same thing and is only spared because a system header does not warn. A real conversion instead.
        constexpr digit_char(unsigned char c) noexcept  // NOLINT(misc-explicit-constructor,google-explicit-constructor,hicpp-explicit-conversions)
        :
                v(c)
        {}

        [[nodiscard]] friend constexpr auto operator==(digit_char, digit_char) noexcept -> bool = default;
};

} // namespace test_chars

using test_chars::digit_char;

// NOLINTBEGIN(bugprone-std-namespace-modification,cert-dcl58-cpp): an explicit specialization for a program-defined type is what the standard invites here.
template<>
struct std::char_traits<digit_char>
{
        using char_type  = digit_char;
        using int_type   = int;
        using off_type   = std::streamoff;
        using pos_type   = std::streampos;
        using state_type = std::mbstate_t;
        using comparison_category = std::strong_ordering;

        static constexpr auto assign(char_type& a, char_type const& b) noexcept -> void { a = b; }
        static constexpr auto eq(char_type a, char_type b) noexcept -> bool { return a.v == b.v; }
        static constexpr auto lt(char_type a, char_type b) noexcept -> bool { return a.v <  b.v; }

        static constexpr auto compare(char_type const* a, char_type const* b, std::size_t n) noexcept -> int
        {
                for (auto i = 0UZ; i < n; ++i) {
                        if (lt(a[i], b[i])) { return -1; }
                        if (lt(b[i], a[i])) { return  1; }
                }
                return 0;
        }

        static constexpr auto length(char_type const* p) noexcept -> std::size_t
        {
                auto n = 0UZ;
                while (p[n].v != 0) { ++n; }
                return n;
        }

        static constexpr auto find(char_type const* p, std::size_t n, char_type const& a) noexcept -> char_type const*
        {
                for (auto i = 0UZ; i < n; ++i) {
                        if (eq(p[i], a)) { return p + i; }
                }
                return nullptr;
        }

        static constexpr auto move(char_type* d, char_type const* s, std::size_t n) noexcept -> char_type*
        {
                if (d < s) {
                        for (auto i = 0UZ; i < n; ++i) { d[i] = s[i]; }
                } else if (s < d) {
                        for (auto i = n; i > 0UZ; --i) { d[i - 1] = s[i - 1]; }
                }
                return d;
        }

        static constexpr auto copy(char_type* d, char_type const* s, std::size_t n) noexcept -> char_type*
        {
                for (auto i = 0UZ; i < n; ++i) { d[i] = s[i]; }
                return d;
        }

        static constexpr auto assign(char_type* p, std::size_t n, char_type a) noexcept -> char_type*
        {
                for (auto i = 0UZ; i < n; ++i) { p[i] = a; }
                return p;
        }

        static constexpr auto not_eof(int_type c) noexcept -> int_type { return c == eof() ? 0 : c; }
        static constexpr auto to_char_type(int_type c) noexcept -> char_type { return { static_cast<unsigned char>(c) }; }
        static constexpr auto to_int_type(char_type c) noexcept -> int_type { return c.v; }
        static constexpr auto eq_int_type(int_type a, int_type b) noexcept -> bool { return a == b; }
        static constexpr auto eof() noexcept -> int_type { return -1; }
};
// NOLINTEND(bugprone-std-namespace-modification,cert-dcl58-cpp)

BOOST_AUTO_TEST_SUITE(BitsetAdaptor)

// Dependent, so an unsatisfied class constraint is a false rather than a hard error; an expression rather than a type requirement, which clang-tidy 22 misreads.
template<class B>
constexpr bool wrappable = requires { sizeof(xstd::bitset_adaptor<B>); };

// Dependent likewise, so a storage without an allocator answers false; the alias spells the typedef without a typename, which clang-tidy 22 reads as redundant.
template<class X>
using allocator_of = X::allocator_type;

template<class X>
constexpr bool has_allocator = requires (X const& x) { sizeof(allocator_of<X>); x.get_allocator(); };

// Only our storages can be wrapped, and the refusal is nominal now rather than a vocabulary the candidate fails to speak.
BOOST_AUTO_TEST_CASE(TheWrappedStoragesAreOursAndTheCounterpartsAreNot)
{
        static_assert(wrappable<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 0>>);
        static_assert(wrappable<xstd::detail::bits::contiguous_bit_array<std::uint64_t, 100>>);
        static_assert(wrappable<xstd::detail::bits::contiguous_bit_vector<std::size_t>>);

        // Not for want of the vocabulary: boost::dynamic_bitset speaks all of it and is still refused, because being ours is the question and not what a candidate's members answer.
        static_assert(not wrappable<std::bitset<64>>);
        static_assert(not wrappable<boost::dynamic_bitset<>>);
        static_assert(not wrappable<std::vector<bool>>);
        static_assert(not wrappable<std::vector<std::uint8_t>>);
}

// The public name is the wrapper over a packed array, with the word type in the open.
BOOST_AUTO_TEST_CASE(TheBitsetIsTheWrapperOverAPackedArray)
{
        static_assert(std::same_as<xstd::basic_bitset<std::uint8_t, 9>, xstd::bitset_adaptor<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 9>>>);
        static_assert(std::same_as<xstd::bitset<64>, xstd::bitset_adaptor<xstd::detail::bits::contiguous_bit_array<std::size_t, 64>>>);
}

using Static = std::tuple
<       xstd::basic_bitset<std::uint8_t, 0>
,       xstd::basic_bitset<std::uint8_t, 1>
,       xstd::basic_bitset<std::uint8_t, 64>
,       xstd::basic_bitset<std::uint8_t, 65>
,       xstd::basic_bitset<std::uint8_t, 128>
,       xstd::bitset<  0>
,       xstd::bitset< 64>
,       xstd::bitset< 65>
>;

// A regular, nothrow, trivially copyable type that is not a range: what std::bitset is, and what a strict extension keeps.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitsetHasStdBitsetsShape, T, Static)
{
        static_assert(std::regular<T>);
        static_assert(not std::ranges::range<T>);
        static_assert(std::is_nothrow_default_constructible_v<T>);
        static_assert(std::is_nothrow_copy_constructible_v<T>);
        static_assert(std::is_trivially_copy_constructible_v<T>);
        static_assert(std::is_trivially_copy_assignable_v<T>);
        static_assert(std::is_trivially_destructible_v<T>);
        static_assert(std::ranges::bidirectional_range<decltype(xstd::bit_set_view(std::declval<T&>()))>);
        static_assert(std::ranges::random_access_range<decltype(xstd::bit_span(std::declval<T&>()))>);
}

using Ours = xstd::basic_bitset<std::uint8_t, 9>;

// Member by member, ours answers exactly as std::bitset does, throw for throw.
BOOST_AUTO_TEST_CASE(OursAnswersAsStdBitsetDoes)
{
        auto w = Ours();
        auto s = std::bitset<9>();
        BOOST_CHECK_EQUAL(w.size(), s.size());
        BOOST_CHECK(w.none() and s.none());

        w.set(3); s.set(3);
        w.set(8, true); s.set(8, true);
        BOOST_CHECK_EQUAL(w.count(), s.count());
        BOOST_CHECK_EQUAL(w.test(3), s.test(3));
        BOOST_CHECK_EQUAL(w[8], s[8]);
        BOOST_CHECK_EQUAL(w.to_string(), s.to_string());

        w.flip(3); s.flip(3);
        w.reset(8); s.reset(8);
        BOOST_CHECK(w.none() and s.none());
}

// The checked family throws where std::bitset throws: one guard, at a static width.
BOOST_AUTO_TEST_CASE(TheCheckedFamilyThrowsAsStdBitsetDoes)
{
        auto w = Ours();
        BOOST_CHECK_THROW(w.set(9), std::out_of_range);
        BOOST_CHECK_THROW(w.reset(9), std::out_of_range);
        BOOST_CHECK_THROW(w.flip(9), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(w.test(9)), std::out_of_range);
}

// The shifts are total on both counterparts, saturating to none, and the derived operators compose on a copy.
BOOST_AUTO_TEST_CASE(TheShiftsSaturateAsStdBitsetDoes)
{
        auto w = Ours();
        auto s = std::bitset<9>();

        w.set(); s.set();
        w <<= 4; s <<= 4;
        BOOST_CHECK_EQUAL(w.to_string(), s.to_string());
        w >>= 9; s >>= 9;
        BOOST_CHECK(w.none() and s.none());
        w.set(0); s.set(0);
        w <<= 9; s <<= 9;
        BOOST_CHECK(w.none() and s.none());

        w.flip(); s.flip();
        BOOST_CHECK_EQUAL((~w).count(), (~s).count());
        BOOST_CHECK_EQUAL((w << 1).to_string(), (s << 1).to_string());
        BOOST_CHECK_EQUAL((w >> 1).to_string(), (s >> 1).to_string());
}

// The proxy writes and reads through the trait, and swaps as a value.
BOOST_AUTO_TEST_CASE(TheProxyWritesThrough)
{
        auto w = xstd::basic_bitset<std::uint8_t, 8>();

        w[3] = true;
        BOOST_CHECK(w[3] and w.test(3));
        w[2] = w[3];
        BOOST_CHECK(w[2]);
        BOOST_CHECK_EQUAL(~w[2], false);
        w[2].flip();
        BOOST_CHECK(not w[2]);

        auto const x = w[2];
        auto const y = w[3];
        swap(x, y);
        BOOST_CHECK(w[2] and not w[3]);

        auto z = true;
        swap(w[3], z);
        BOOST_CHECK(w[3] and not z);
        swap(z, w[3]);
        BOOST_CHECK(z and not w[3]);
}

// The set vocabulary boost has and std::bitset has not is there at a static width too, and so are the two searches.
BOOST_AUTO_TEST_CASE(TheExtensionIsThereAtAStaticWidth)
{
        auto d = Ours(0b101ULL);
        auto e = Ours();
        e.set(2);

        BOOST_CHECK(e.is_subset_of(d));
        BOOST_CHECK(e.is_proper_subset_of(d));
        BOOST_CHECK(d.intersects(e));
        BOOST_CHECK(not e.is_proper_subset_of(e));
        BOOST_CHECK(not Ours().intersects(d));

        // The symmetric spelling beside boost's member, both orders alike.
        BOOST_CHECK(intersects(d, e) and intersects(e, d));
        BOOST_CHECK(not intersects(Ours(), d) and not intersects(d, Ours()));

        BOOST_CHECK_EQUAL(d.find_first(), 0UZ);
        BOOST_CHECK_EQUAL(d.find_next(0), 2UZ);
        BOOST_CHECK_EQUAL(d.find_next(2), Ours::npos);
        BOOST_CHECK_EQUAL(Ours().find_first(), Ours::npos);
        using Empty = xstd::basic_bitset<std::uint8_t, 0>;
        BOOST_CHECK_EQUAL(Empty().find_first(), Empty::npos);
        BOOST_CHECK_EQUAL(Empty().find_next(0), Empty::npos);

        auto const f = d - e;
        d -= e;
        BOOST_CHECK(f == d);
        BOOST_CHECK_EQUAL(d.count(), 1UZ);
        BOOST_CHECK(d.test(0) and not d.test(2));
}

// The forward pair answers for the positions the reverse pair below was already asked about, and did not: boost's find_next is total -- `if (pos >= sz - 1 || sz == 0) return npos` -- where the storage's step asserts is_valid(n) and moves to n + 1. So a pos past the width read a block index the storage need not have, and find_next(npos) was the worst of it: n + 1 wraps to zero, the scan starts at the beginning, and the answer is the FIRST set position rather than none.
BOOST_AUTO_TEST_CASE(TheForwardSearchesAreTotalPastTheWidth)
{
        auto const d = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(d.find_next(1), 2UZ);
        BOOST_CHECK_EQUAL(d.find_next(8), Ours::npos);          // the last position this width has
        BOOST_CHECK_EQUAL(d.find_next(9), Ours::npos);          // the first it has not
        BOOST_CHECK_EQUAL(d.find_next(100), Ours::npos);
        BOOST_CHECK_EQUAL(d.find_next(Ours::npos), Ours::npos);
        BOOST_CHECK_EQUAL(Ours().find_next(0), Ours::npos);

        // And at a width of more than one block, which is where the walk below steps through find_next: the last block is the one a step past the width would read beyond.
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
        auto w = Wide();
        w.set(69);
        BOOST_CHECK_EQUAL(w.find_next(68), 69UZ);
        BOOST_CHECK_EQUAL(w.find_next(69), Wide::npos);
        BOOST_CHECK_EQUAL(w.find_next(70), Wide::npos);
        BOOST_CHECK_EQUAL(w.find_next(Wide::npos), Wide::npos);
}

// The reverse pair mirrors boost's forward pair: the highest set position below pos, npos where none, a pos past the width meaning from the end.
BOOST_AUTO_TEST_CASE(TheReverseSearchesMirrorTheForwardOnes)
{
        auto const d = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(d.find_last(), 2UZ);
        BOOST_CHECK_EQUAL(d.find_prev(2), 0UZ);
        BOOST_CHECK_EQUAL(d.find_prev(1), 0UZ);
        BOOST_CHECK_EQUAL(d.find_prev(0), Ours::npos);
        BOOST_CHECK_EQUAL(d.find_prev(100), 2UZ);
        BOOST_CHECK_EQUAL(d.find_prev(Ours::npos), d.find_last());
        BOOST_CHECK_EQUAL(Ours().find_last(), Ours::npos);

        using Empty = xstd::basic_bitset<std::uint8_t, 0>;
        BOOST_CHECK_EQUAL(Empty().find_last(), Empty::npos);
        BOOST_CHECK_EQUAL(Empty().find_prev(0), Empty::npos);

        // The two loops are each other's reverse, across blocks.
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
        auto w = Wide();
        w.set(1); w.set(8); w.set(9); w.set(69);
        auto forward = std::vector<std::size_t>();
        for (auto i = w.find_first(); i != Wide::npos; i = w.find_next(i)) {
                forward.push_back(i);
        }
        auto backward = std::vector<std::size_t>();
        for (auto i = w.find_last(); i != Wide::npos; i = w.find_prev(i)) {
                backward.push_back(i);
        }
        BOOST_CHECK((forward == std::vector<std::size_t>{ 1, 8, 9, 69 }));
        BOOST_CHECK(std::ranges::equal(forward, std::views::reverse(backward)));
}

// The three walks over one bitset answer the same positions in the same order. Iteration and for_each were already held to each other, in the set reading's own tests; what is new here is the SCAN beside them, because that is the walk a benchmark puts against boost's find_first/find_next, and a ratio between two scans is a cost only while both answer the same thing.
// It is also the walk that can disagree. The other two carry their own state -- an index, a block with the reported bit cleared -- where every step of the scan is a fresh entry at a position, and find_next's npos guard is the whole of what keeps a step past the last set position from wrapping round to the first.
template<class T>
auto scanned(T const& b)
        -> std::vector<std::size_t>
{
        auto v = std::vector<std::size_t>();
        for (auto pos = b.find_first(); pos != T::npos; pos = b.find_next(pos)) {
                v.push_back(pos);
        }
        return v;
}

template<class T>
auto walks_agree(T const& b)
        -> bool
{
        auto iterated = std::vector<std::size_t>();
        for (auto const pos : xstd::bit_set_view(b)) {
                iterated.push_back(pos);
        }
        auto blockwise = std::vector<std::size_t>();
        xstd::bit_set_view(b).for_each([&blockwise](std::size_t pos) -> void { blockwise.push_back(pos); });
        auto const scan = scanned(b);
        return scan == iterated and scan == blockwise and scan.size() == b.count();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheScanWalksWhatIterationWalks, T, Static)
{
        auto const none = T();
        BOOST_CHECK(walks_agree(none));

        auto all = T();
        all.set();
        BOOST_CHECK(walks_agree(all));

        // One position at a time, the last one included: that is the step whose next entry is past the width, and the only one at a width of one.
        for (auto const i : std::views::iota(0UZ, all.size())) {
                auto one = T();
                one.set(i);
                BOOST_CHECK(walks_agree(one));
        }

        // A stride that leaves whole blocks empty between set positions, which is where the scan walks blocks rather than bits.
        auto sparse = T();
        for (auto i = 0UZ; i < sparse.size(); i += 17UZ) {
                sparse.set(i);
        }
        BOOST_CHECK(walks_agree(sparse));
}

// The same at a run-time width, which is the width the ladder in benchmark/src/bitset/dynamic.cpp measures and the one whose scan takes the general arm rather than a one- or two-block unrolling.
BOOST_AUTO_TEST_CASE(TheScanWalksWhatIterationWalksAtARunTimeWidth)
{
        for (auto const n : { 0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 129UZ, 512UZ }) {
                auto const none = xstd::dynamic_bitset(n);
                BOOST_CHECK(walks_agree(none));

                auto all = xstd::dynamic_bitset(n);
                all.set();
                BOOST_CHECK(walks_agree(all));

                auto sparse = xstd::dynamic_bitset(n);
                for (auto i = 0UZ; i < n; i += 17UZ) {
                        sparse.set(i);
                }
                BOOST_CHECK(walks_agree(sparse));
        }
}

// The ordering is the bit string's, to_string() compared, which within a word is the number's.
BOOST_AUTO_TEST_CASE(TheOrderingIsTheBitStrings)
{
        static_assert(std::totally_ordered<Ours>);
        for (auto const a : std::views::iota(0ULL, 512ULL)) {
                for (auto const b : std::views::iota(0ULL, 512ULL)) {
                        auto const x = Ours(a);
                        auto const y = Ours(b);
                        BOOST_CHECK((x <=> y) == (a <=> b));
                        BOOST_CHECK((x <=> y) == (x.to_string() <=> y.to_string()));
                }
        }

        // Across blocks: the top block decides before the lower ones say anything.
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
        auto top = Wide();
        top.set(69);
        auto rest = Wide();
        rest.set();
        rest.reset(69);
        BOOST_CHECK(rest < top);
        BOOST_CHECK((Wide() <=> Wide()) == std::strong_ordering::equal);
        BOOST_CHECK(Wide() < top);
}

// LWG 4294's Constraints, which is what the const charT* constructor is held to now: the four char-like traits, and
// not a list of the five types the standard specializes char_traits for. A program-defined char-like type reaches
// this constructor exactly as it reaches std::bitset's.
//
// One clause is ours and not the standard's, because std::bitset has no overload to be told apart from: a pointer to
// a BLOCK is the block-range constructor's argument. Asserted here so the widening above cannot quietly swallow it.
BOOST_AUTO_TEST_CASE(TheStringConstructorTakesAnyCharLikeTypeButABlock)
{
        static_assert(std::is_constructible_v<Ours, char const*>);
        static_assert(std::is_constructible_v<Ours, wchar_t const*>);
        static_assert(std::is_constructible_v<Ours, char8_t const*>);
        static_assert(std::is_constructible_v<Ours, char16_t const*>);
        static_assert(std::is_constructible_v<Ours, char32_t const*>);

        // CALLED, and not merely asked about: is_constructible_v answers from the declaration and never instantiates
        // the body, so the five above passed for years while the error path inside was ill-formed for four of them.
        // std::format wants a formatter<charT, char> and the standard specializes formatter<charT, charT>; there is
        // no formatter<wchar_t, char>. One construction per character type is what finds that, and one throw per
        // character type is what reaches the message.
        BOOST_CHECK(Ours( "101") == Ours("101"));
        BOOST_CHECK(Ours(L"101") == Ours("101"));
        BOOST_CHECK(Ours(u8"101") == Ours("101"));
        BOOST_CHECK(Ours(u"101") == Ours("101"));
        BOOST_CHECK(Ours(U"101") == Ours("101"));

        BOOST_CHECK_THROW(static_cast<void>(Ours( "102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(L"102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(u8"102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(u"102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(U"102")), std::invalid_argument);

        // The widening: not one of the five, and constructible all the same.
        static_assert(std::is_constructible_v<Ours, digit_char const*>);
        static_assert(std::is_constructible_v<std::bitset<9>, digit_char const*>);

        constexpr auto zero = digit_char{ static_cast<unsigned char>('0') };
        constexpr auto one  = digit_char{ static_cast<unsigned char>('1') };
        auto const text = std::array<digit_char, 4>{ one, zero, one, digit_char{ 0 } };
        BOOST_CHECK(Ours(text.data(), std::basic_string_view<digit_char>::npos, zero, one) == Ours("101"));

        // And its error path, which is the third arm: char-like, and neither a character nor a number to a narrow
        // format string. A program-defined char-like type is exactly what LWG 4294's Constraints let in.
        auto const bad = std::array<digit_char, 4>{ one, digit_char{ static_cast<unsigned char>('2') }, one, digit_char{ 0 } };
        BOOST_CHECK_THROW(static_cast<void>(Ours(bad.data(), std::basic_string_view<digit_char>::npos, zero, one)), std::invalid_argument);

        // The one subtraction, and the reason for it: the block-range constructor keeps its argument.
        static_assert(std::same_as<Ours::block_type, std::uint8_t>);
        static_assert(not std::is_constructible_v<Ours, std::uint8_t const*>);
}

// boost's block interface: the block type and its width, the block count, every block out and at most every block in, the tail kept clear.
BOOST_AUTO_TEST_CASE(TheBlockInterfaceIsBoosts)
{
        static_assert(std::same_as<Ours::block_type, std::uint8_t>);
        static_assert(Ours::bits_per_block == 8UZ);
        BOOST_CHECK_EQUAL(Ours().num_blocks(), 2UZ);

        auto const b = Ours("101000001");
        auto out = std::vector<std::uint8_t>();
        to_block_range(b, std::back_inserter(out));
        BOOST_CHECK((out == std::vector<std::uint8_t>{ 0b0100'0001, 0b1 }));

        auto c = Ours();
        from_block_range(out.begin(), out.end(), c);
        BOOST_CHECK(c == b);

        // Fewer blocks than there are leaves the rest alone; a dirty tail is masked rather than kept.
        auto const dirty = std::array<std::uint8_t, 2>{ 0b1000'0000, 0b1111'1111 };
        auto e = Ours();
        from_block_range(dirty.begin(), dirty.end(), e);
        BOOST_CHECK_EQUAL(e.count(), 2UZ);
        BOOST_CHECK(e.test(7) and e.test(8));
        from_block_range(dirty.begin(), dirty.begin() + 1, c);
        BOOST_CHECK_EQUAL(c.count(), 2UZ);
        BOOST_CHECK(c.test(7) and c.test(8));
}

// from_block_range copies the span where its source is contiguous and sized, and loops where it is not. Every caller above hands it a vector or an array, so the loop is the arm that would otherwise stop being exercised the day the copy was added -- and the two arms answering differently is the only way this change can be wrong.
BOOST_AUTO_TEST_CASE(TheTwoArmsOfFromBlockRangeAgree)
{
        // A list is neither contiguous nor sized against its sentinel, so it takes the loop; the same blocks out of a vector take the copy.
        static_assert(not std::contiguous_iterator<std::list<std::uint8_t>::const_iterator>);
        static_assert(std::contiguous_iterator<std::vector<std::uint8_t>::const_iterator>);

        auto const sources = std::vector<std::vector<std::uint8_t>>{
                {},                                             // nothing named leaves the target alone
                { 0b1010'0101 },                                // fewer blocks than there are
                { 0b1010'0101, 0b1 },
                { 0b0000'0000, 0b0 },
                { 0b1111'1111, 0b1111'1111 },                   // a dirty tail, masked by both arms alike
        };
        for (auto const& blocks : sources) {
                auto const as_list = std::list<std::uint8_t>(blocks.begin(), blocks.end());

                // A non-empty starting value, so "leaves the rest alone" is something the check can see.
                auto copied = Ours("101000001");
                auto looped = Ours("101000001");
                from_block_range(blocks.begin(), blocks.end(), copied);
                from_block_range(as_list.begin(), as_list.end(), looped);

                BOOST_CHECK(copied == looped);

                auto out_copied = std::vector<std::uint8_t>();
                auto out_looped = std::vector<std::uint8_t>();
                to_block_range(copied, std::back_inserter(out_copied));
                to_block_range(looped, std::back_inserter(out_looped));
                BOOST_CHECK(out_copied == out_looped);
        }
}

// The same question for the way out, which now has two arms of its own: a vector's iterator is contiguous and takes the copy, a back_inserter and a list's iterator are not and take the loop. Nine bits over eight-bit blocks, so the last block is seven-eighths unused, and the patterns are the ones that put something interesting there: all clear, all set, and a bit at each end. boost's contract is that every block comes out including that tail, and the two arms have to agree about it.
BOOST_AUTO_TEST_CASE(TheTwoArmsOfToBlockRangeAgree)
{
        static_assert(std::contiguous_iterator<std::vector<std::uint8_t>::iterator>);
        static_assert(not std::contiguous_iterator<std::back_insert_iterator<std::vector<std::uint8_t>>>);
        static_assert(not std::contiguous_iterator<std::list<std::uint8_t>::iterator>);

        auto const patterns = std::vector<std::string>{
                "000000000", "111111111", "101000001", "100000000", "000000001",
        };
        for (auto const& pattern : patterns) {
                auto const b = Ours(pattern);

                // The contiguous arm, into a buffer the caller sized.
                auto copied = std::vector<std::uint8_t>(b.num_blocks());
                to_block_range(b, copied.begin());

                // The loop, twice over, through two output iterators that are not contiguous for different reasons.
                auto appended = std::vector<std::uint8_t>();
                to_block_range(b, std::back_inserter(appended));
                auto listed = std::list<std::uint8_t>(b.num_blocks());
                to_block_range(b, listed.begin());

                BOOST_CHECK(copied == appended);
                BOOST_CHECK(std::ranges::equal(copied, listed));

                // And the blocks that came out still name the bits that went in.
                auto round_trip = Ours();
                from_block_range(copied.begin(), copied.end(), round_trip);
                BOOST_CHECK(round_trip == b);
        }
}

// boost's remaining members at a static width, where every guard throws: at, test_set, the ranged forms, max_size; no allocator, the storage having none.
BOOST_AUTO_TEST_CASE(TheRestOfBoostsSurfaceIsThereAtAStaticWidth)
{
        static_assert(not has_allocator<Ours>);
        static_assert(has_allocator<xstd::basic_dynamic_bitset<std::uint8_t>>);
        BOOST_CHECK_EQUAL(Ours().max_size(), 9UZ);

        auto d = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(d.at(0), true);
        BOOST_CHECK_EQUAL(std::as_const(d).at(1), false);
        d.at(1) = true;
        BOOST_CHECK(d.test(1));
        BOOST_CHECK_THROW(static_cast<void>(d.at(9)), std::out_of_range);
        BOOST_CHECK_THROW(static_cast<void>(std::as_const(d).at(9)), std::out_of_range);

        BOOST_CHECK_EQUAL(d.test_set(1, false), true);
        BOOST_CHECK_EQUAL(d.test_set(1), false);
        BOOST_CHECK(d.test(1));
        BOOST_CHECK_THROW(static_cast<void>(d.test_set(9)), std::out_of_range);

        d.set(3, 5, true);
        BOOST_CHECK_EQUAL(d.to_ullong(), 0b1111'1111ULL - 0b100ULL + 0b100ULL);
        d.reset(0, 2);
        BOOST_CHECK_EQUAL(d.to_ullong(), 0b1111'1100ULL);
        d.flip(0, 9);
        BOOST_CHECK_EQUAL(d.to_ullong(), 0b1'0000'0011ULL);
        d.set(9, 0, true);
        BOOST_CHECK_THROW(d.set(8, 2, true), std::out_of_range);
        BOOST_CHECK_THROW(d.reset(9, 1), std::out_of_range);
        BOOST_CHECK_THROW(d.flip(5, 5), std::out_of_range);

        // pos + len is the sum the guard refuses to make: near the top of size_t it wraps to a value below every width, so the check the range was meant to fail is the one it would pass, and the ranged forms would then write nothing and report nothing.
        constexpr auto top = std::numeric_limits<std::size_t>::max();
        BOOST_CHECK_THROW(d.set(top, 1, true), std::out_of_range);
        BOOST_CHECK_THROW(d.reset(top - 3, 8), std::out_of_range);
        BOOST_CHECK_THROW(d.flip(top, top), std::out_of_range);

        // A position past the width is out of range whatever the length, and a refused range writes nothing.
        BOOST_CHECK_THROW(d.set(10, 0, true), std::out_of_range);
        BOOST_CHECK_EQUAL(d.to_ullong(), 0b1'0000'0011ULL);
}

// The views reach a bitset by referring into its storage: the ordering, the keys, the blocks.
BOOST_AUTO_TEST_CASE(TheViewsReachABitset)
{
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
        auto a = Wide();
        auto b = Wide();
        a.set(1); a.set(69);
        b.set(1); b.set(2);

        // Named rather than called on the temporaries: clang 23's lifetime analysis crashes on a deducing-this member of a prvalue.
        auto const va = xstd::bit_set_view(a);
        auto const vb = xstd::bit_set_view(b);
        auto const qa = xstd::bit_span(a);

        auto keys = std::vector<std::size_t>();
        for (auto const k : va) {
                keys.push_back(k);
        }
        BOOST_CHECK((keys == std::vector<std::size_t>{ 1, 69 }));

        BOOST_CHECK(va != vb);
        BOOST_CHECK(std::is_lt(vb <=> va));
        BOOST_CHECK(va.is_subset_of(va));
        BOOST_CHECK_EQUAL(qa[69], true);

        static_assert(std::same_as<decltype(va), xstd::bit_set_view<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 70>> const>);
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::as_const(a))), xstd::bit_set_view<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 70> const>>);
}

// Built from text, streamed back to text, and hashed: the derived members.
BOOST_AUTO_TEST_CASE(TheDerivedMembersHold)
{
        auto const packed = Ours("101000001");
        BOOST_CHECK_EQUAL(packed.to_string(), std::bitset<9>("101000001").to_string());
        BOOST_CHECK_EQUAL(packed.count(), 3UZ);
        BOOST_CHECK(packed.test(0) and packed.test(6) and packed.test(8));

        auto const hash = std::hash<Ours>();
        BOOST_CHECK_EQUAL(hash(packed), hash(Ours("101000001")));
        BOOST_CHECK(hash(packed) != hash(Ours()));

        // A count on the character-pointer form takes that many characters and no more.
        BOOST_CHECK_EQUAL(Ours("1111", 2).count(), 2UZ);
}

// [bitset.cons]/2 and [bitset.members]/34-37: the word in and the word out, the overflow where a set position lies beyond it.
BOOST_AUTO_TEST_CASE(TheWordConstructorAndConversionsAgreeWithStdBitset)
{
        auto const s = std::bitset<9>(0b101ULL);
        auto const p = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(p.to_string(), s.to_string());
        BOOST_CHECK_EQUAL(p.to_ullong(), s.to_ullong());
        BOOST_CHECK_EQUAL(p.to_ulong(), s.to_ulong());

        // The high bits of the value drop where the width is narrower, as [bitset.cons]/2 has it.
        using Narrow = xstd::basic_bitset<std::uint8_t, 3>;
        using Empty  = xstd::basic_bitset<std::uint8_t, 0>;
        BOOST_CHECK_EQUAL(Narrow(0b1111ULL).to_ullong(), 7ULL);
        BOOST_CHECK_EQUAL(Narrow(0b1101ULL).to_ullong(), 5ULL);
        BOOST_CHECK_EQUAL(Empty(0b1111ULL).to_ullong(), 0ULL);

        auto wide = xstd::basic_bitset<std::uint8_t, 70>();
        BOOST_CHECK_EQUAL(wide.to_ullong(), 0ULL);
        wide.set(69);
        BOOST_CHECK_THROW(static_cast<void>(wide.to_ullong()), std::overflow_error);
        BOOST_CHECK_THROW(static_cast<void>(wide.to_ulong()), std::overflow_error);
}

// [bitset.operators]/6: a short read lands in the low bits, as std::bitset(str) puts it, and an empty read fails the stream.
BOOST_AUTO_TEST_CASE(ExtractionOfAShortInputAgreesWithStdBitset)
{
        auto in = std::istringstream("1");
        auto s = std::bitset<4>();
        in >> s;
        auto ours = std::istringstream("1");
        auto x = xstd::basic_bitset<std::uint8_t, 4>();
        ours >> x;
        BOOST_CHECK_EQUAL(x.to_string(), s.to_string());
        BOOST_CHECK_EQUAL(x.to_ullong(), 1ULL);

        auto stop = std::istringstream("01x");
        auto y = xstd::basic_bitset<std::uint8_t, 4>();
        stop >> y;
        BOOST_CHECK_EQUAL(y.to_string(), "0001");
        BOOST_CHECK(not stop.fail());
}

// The text constructors reject as std::bitset's do: a stray character, and a position past the end.
BOOST_AUTO_TEST_CASE(TheTextConstructorsRejectWhatStdBitsetRejects)
{
        BOOST_CHECK_THROW(static_cast<void>(Ours("102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(std::string("101"), 4)), std::out_of_range);
}

// A view over a bitset binds the storage it wraps, and that is the only spelling there is.
BOOST_AUTO_TEST_CASE(ABitsetReadsAsItsStorage)
{
        using B = xstd::bitset<100>;

        // A view over a bitset binds the storage it wraps, which is now the only spelling: naming the bitset itself as a view's Bits is what the constraint refuses, the storage being the thing a view refers into.
        using Blocks = xstd::detail::bits::contiguous_bit_array<std::size_t, 100>;
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::declval<B&>())), xstd::bit_set_view<Blocks>>);
        static_assert(std::same_as<decltype(xstd::bit_span(std::declval<B&>())),     xstd::bit_span<Blocks>>);

        // Naming the bitset changes how a view is spelled, not what the bitset offers.
        static_assert(not std::ranges::range<B>);

        auto bs = B();
        bs.set(3);
        bs.set(41);

        auto const sv = xstd::bit_set_view(bs);
        auto const sp = xstd::bit_span(bs);
        BOOST_CHECK_EQUAL(std::ranges::distance(sv), 2);
        BOOST_CHECK_EQUAL(std::ranges::distance(sp), 100);
        BOOST_CHECK(std::ranges::bidirectional_range<decltype(sv)>);
        BOOST_CHECK(std::ranges::random_access_range<decltype(sp)>);
}

// The byte exchange, at the bitset reading -- the one reading that already had a door for integers and so opens
// only the other one, on container_source rather than on bit_castable.
BOOST_AUTO_TEST_CASE(ABitsetReadingExchangesBytesWithAnotherFieldOfBits)
{
        constexpr auto N = 100UZ;
        auto src = std::bitset<N>();
        for (auto i = 0UZ; i < N; i += 3UZ) { src.set(i); }

        auto const b = xstd::bitset<N>::from_bits(src);
        BOOST_CHECK_EQUAL(b.count(), src.count());
        for (auto i = 0UZ; i < N; ++i) {
                BOOST_CHECK_EQUAL(b.test(i), src.test(i));
        }
        BOOST_CHECK(b.to_bits<std::bitset<N>>() == src);

        static_assert([] -> bool {
                auto const bs = std::bitset<64>(0x0F1E'2D3C'4B5A'6978ULL);
                return xstd::bitset<64>::from_bits(bs).to_bits<std::bitset<64>>() == bs;
        }());
}

// THE INTEGER DOOR IS UNTOUCHED, which is the whole reason this reading is constrained on container_source and not
// on bit_castable. Admitting the integer family would not widen the interface but collide with it, in two ways that
// this case pins down so a later widening cannot pass silently.
BOOST_AUTO_TEST_CASE(TheIntegerDoorIsUnchangedByTheByteExchange)
{
        // [bitset.cons]/2's constructor is IMPLICIT, and an explicit template admitting unsigned int would be an
        // exact match where that one takes a conversion -- so it would win for bitset<32>(5U) and, being explicit,
        // make this copy-initialization ill-formed.
        static_assert(std::is_convertible_v<unsigned long long, xstd::bitset<64>>);
        static_assert(std::is_convertible_v<unsigned,           xstd::bitset<32>>);
        xstd::bitset<32> const implicitly = 5U;
        BOOST_CHECK_EQUAL(implicitly.to_ullong(), 5ULL);

        // And to_ullong keeps ITS contract, which the byte exchange does not share: a set position beyond the word
        // throws where a byte copy would silently keep the low bits.
        auto wide = xstd::bitset<100>();
        wide.set(99);
        BOOST_CHECK_THROW((void)wide.to_ullong(), std::overflow_error);
        // The byte exchange over the same object does not throw, because it is not the same question.
        BOOST_CHECK(wide.to_bits<std::bitset<100>>().test(99));
}

// Two block widths over the same N are two spellings of one field of bits, so they cross on the same rule with
// neither side named: the byte is the common ground where the word is not.
BOOST_AUTO_TEST_CASE(TwoBlockWidthsCrossOnTheSameRule)
{
        static_assert([] -> bool {
                auto const wide   = xstd::basic_bitset<std::uint64_t, 64>::from_bits(std::bitset<64>(0xABCDULL));
                auto const narrow = xstd::basic_bitset<std::uint8_t,  64>::from_bits(wide);
                return narrow.to_bits<std::bitset<64>>() == std::bitset<64>(0xABCDULL);
        }());
}

// Named both ways, any other width no exchange at all, and a run-time width none of it.
BOOST_AUTO_TEST_CASE(TheBitsetExchangeIsNamedAndWidthExact)
{
        constexpr auto N = 64UZ;
        using T = xstd::bitset<N>;
        static_assert(test::exchanges_bits<T, std::bitset<N>>);

        // The UNNAMED doors are closed, both of them, which is what the rename is for. Worth asserting rather than
        // assuming: a conversion operator reintroduced later would be invisible to the concept above, since that
        // one only asks whether the named spelling works.
        static_assert(not std::is_constructible_v<T, std::bitset<N>>);
        static_assert(not std::is_constructible_v<std::bitset<N>, T>);
        static_assert(not std::is_convertible_v  <std::bitset<N>, T>);
        static_assert(not std::is_convertible_v  <T, std::bitset<N>>);

        // ITS width, not merely one that fits, in the direction the width is checked.
        static_assert(not test::exchanges_from_bits<T, std::bitset<N + 1UZ>>);
        static_assert(not test::exchanges_from_bits<T, std::bitset<N - 1UZ>>);

        using Dynamic = xstd::basic_dynamic_bitset<std::uint64_t>;
        static_assert(not test::exchanges_from_bits<Dynamic, std::bitset<N>>);
        static_assert(not test::exchanges_to_bits  <Dynamic, std::bitset<N>>);
}

// A SEQUENCE OF BLOCKS is a field of bits, so this reading takes it -- unlike the bare scalar, which it declines
// because [bitset.cons]/2 and to_ullong already own that door. Nothing about blocks collides with the standard's
// interface, so nothing is left out.
BOOST_AUTO_TEST_CASE(ASequenceOfBlocksIsAFieldOfBitsAndAScalarIsNot)
{
        using Blocks = std::array<std::uint64_t, 2>;
        static_assert(test::exchanges_bits<xstd::bitset<128>, Blocks>);

        // AND A SCALAR IS NOT, which this reading can now say in one line about the exchange itself. Under the old
        // spelling the same question could only be put to is_constructible_v, where the standard's own implicit
        // integer constructor answers yes and says nothing about the byte exchange at all.
        static_assert(not test::exchanges_from_bits<xstd::bitset<128>, unsigned long long>);
        static_assert(not test::exchanges_to_bits  <xstd::bitset<128>, unsigned long long>);

        // The scalar door stays the standard's, implicit and throwing, rather than a byte copy.
        static_assert(std::is_convertible_v<unsigned long long, xstd::bitset<128>>);

        static_assert([] -> bool {
                auto const b = Blocks{ 0x0123'4567'89AB'CDEFULL, 0xFEDC'BA98'7654'3210ULL };
                return xstd::bitset<128>::from_bits(b).to_bits<Blocks>() == b;
        }());
}

BOOST_AUTO_TEST_SUITE_END()

// contiguous_bit_sequence is structural and says so: it asks the positional members -- test(n), set(n), reset(n), flip(n) -- and every field of bits that has them answers, ours and the counterparts alike. That is a different question from which storages this library wraps, which is nominal and asked by the constraint above.
BOOST_AUTO_TEST_SUITE(TheStructuralQuestionIsNotTheNominalOne)

static_assert(xstd::contiguous_bit_sequence<xstd::detail::bits::contiguous_bit_array<std::uint64_t, 64>>);
static_assert(xstd::contiguous_bit_sequence<boost::dynamic_bitset<>>);
static_assert(xstd::contiguous_bit_sequence<std::bitset<64>>);

BOOST_AUTO_TEST_SUITE_END()
