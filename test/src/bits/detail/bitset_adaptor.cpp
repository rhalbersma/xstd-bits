//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bit_exchange.hpp>                      // casts_between, casts_from, exchanges_bits, exchanges_from_bits, exchanges_to_bits
#include <test/bitset/vocabulary.hpp>                 // vocabulary
#include <xstd/bits/bit/bit_cast.hpp>                 // bit_cast
#include <xstd/bits/bit_set_view.hpp>                 // bit_set_view
#include <xstd/bits/bit_span.hpp>                     // bit_span
#include <xstd/bits/bitset.hpp>                       // basic_bitset, bitset
#include <xstd/bits/detail/bitset_adaptor.hpp>        // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_array.hpp>  // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/dynamic_bitset.hpp>               // basic_dynamic_bitset
#include <xstd/bits/from_bit_storage.hpp>             // from_bit_storage
#include <boost/dynamic_bitset.hpp>                   // dynamic_bitset
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                  // equal
#include <array>                                      // array
#include <bitset>                                     // bitset
#include <compare>                                    // is_lt, strong_ordering
#include <concepts>                                   // regular, same_as, totally_ordered
#include <cstddef>                                    // size_t
#include <cwchar>                                     // mbstate_t
#include <cstdint>                                    // uint8_t, uint64_t
#include <functional>                                 // hash
#include <ios>                                        // streamoff
#include <iosfwd>                                     // streampos
#include <iterator>                                   // back_inserter, contiguous_iterator
#include <limits>                                     // numeric_limits
#include <list>                                       // list
#include <ranges>                                     // equal, iota, range, reverse
#include <sstream>                                    // istringstream
#include <stdexcept>                                  // invalid_argument, out_of_range, overflow_error
#include <string>                                     // char_traits, string
#include <string_view>                                // basic_string_view
#include <tuple>                                      // tuple
#include <type_traits>                                // is_constructible_v, is_convertible_v, is_nothrow_*, is_trivially_*
#include <utility>                                    // as_const, declval
#include <vector>                                     // vector

// A program-defined char-like type, in a named namespace so its char_traits members have external linkage.
namespace test_chars {

struct digit_char
{
        // No default member initializer: one would break LWG 4294's trivially-default-constructible trait.
        unsigned char v;

        // Defaulted on first declaration, so the default constructor below leaves it trivial and all four traits hold.
        digit_char() noexcept = default;

        // A converting constructor: charT('0') on a bare aggregate is a C++20 extension clang diagnoses.
        constexpr digit_char(unsigned char c) noexcept // NOLINT(misc-explicit-constructor,google-explicit-constructor,hicpp-explicit-conversions)
                : v(c)
        {}

        [[nodiscard]] friend auto operator==(digit_char, digit_char) noexcept -> bool = default;
};

} // namespace test_chars

using test_chars::digit_char;

// NOLINTBEGIN(bugprone-std-namespace-modification,cert-dcl58-cpp): the standard invites this specialization.
template<>
struct std::char_traits<digit_char>
{
        using char_type = digit_char;
        using int_type = int;
        using off_type = std::streamoff;
        using pos_type = std::streampos;
        using state_type = std::mbstate_t;
        using comparison_category = std::strong_ordering;

        static constexpr auto assign(char_type& a, char_type const& b) noexcept
                -> void
        {
                a = b;
        }

        static constexpr auto eq(char_type a, char_type b) noexcept
                -> bool
        {
                return a.v == b.v;
        }

        static constexpr auto lt(char_type a, char_type b) noexcept
                -> bool
        {
                return a.v < b.v;
        }

        static constexpr auto compare(char_type const* a, char_type const* b, std::size_t n) noexcept
                -> int
        {
                for (auto const i : std::views::iota(0UZ, n)) {
                        if (lt(a[i], b[i])) {
                                return -1;
                        }
                        if (lt(b[i], a[i])) {
                                return 1;
                        }
                }
                return 0;
        }

        static constexpr auto length(char_type const* p) noexcept
                -> std::size_t
        {
                auto n = 0UZ;
                while (p[n].v != 0) {
                        ++n;
                }
                return n;
        }

        static constexpr auto find(char_type const* p, std::size_t n, char_type const& a) noexcept
                -> char_type const*
        {
                for (auto const i : std::views::iota(0UZ, n)) {
                        if (eq(p[i], a)) {
                                return p + i;
                        }
                }
                return nullptr;
        }

        static constexpr auto move(char_type* d, char_type const* s, std::size_t n) noexcept
                -> char_type*
        {
                if (d < s) {
                        for (auto const i : std::views::iota(0UZ, n)) {
                                d[i] = s[i];
                        }
                } else if (s < d) {
                        for (auto i = n - 1UZ; i < n; --i) {
                                d[i] = s[i];
                        }
                }
                return d;
        }

        static constexpr auto copy(char_type* d, char_type const* s, std::size_t n) noexcept
                -> char_type*
        {
                for (auto const i : std::views::iota(0UZ, n)) {
                        d[i] = s[i];
                }
                return d;
        }

        static constexpr auto assign(char_type* p, std::size_t n, char_type a) noexcept
                -> char_type*
        {
                for (auto const i : std::views::iota(0UZ, n)) {
                        p[i] = a;
                }
                return p;
        }

        static constexpr auto not_eof(int_type c) noexcept
                -> int_type
        {
                return c == eof() ? 0 : c;
        }

        static constexpr auto to_char_type(int_type c) noexcept
                -> char_type
        {
                return {static_cast<unsigned char>(c)};
        }

        static constexpr auto to_int_type(char_type c) noexcept
                -> int_type
        {
                return c.v;
        }

        static constexpr auto eq_int_type(int_type a, int_type b) noexcept
                -> bool
        {
                return a == b;
        }

        static constexpr auto eof() noexcept
                -> int_type
        {
                return -1;
        }
};
// NOLINTEND(bugprone-std-namespace-modification,cert-dcl58-cpp)

BOOST_AUTO_TEST_SUITE(BitsetAdaptor)

// The derived type the vehicle hands its results back as; only ever named, never completed.
struct derived_probe;

// Dependent, so an unsatisfied class constraint is a false rather than a hard error.
template<class B>
constexpr bool wrappable = requires { sizeof(xstd::bits::detail::bitset_adaptor<B, derived_probe>); };

// Dependent likewise, so a storage without an allocator answers false rather than hard-errors.
template<class X>
using allocator_of = X::allocator_type;

template<class X>
constexpr bool has_allocator = requires (X const& x) { sizeof(allocator_of<X>); x.get_allocator(); };

// Only our storages can be wrapped, and the refusal is nominal rather than about vocabulary.
BOOST_AUTO_TEST_CASE(TheWrappedStoragesAreOursAndTheCounterpartsAreNot)
{
        static_assert(wrappable<xstd::bits::detail::contiguous_bit_array<std::uint8_t, 0>>);
        static_assert(wrappable<xstd::bits::detail::contiguous_bit_array<std::uint64_t, 100>>);
        static_assert(wrappable<xstd::bits::detail::contiguous_bit_vector<std::size_t>>);

        // Not for want of the vocabulary: boost::dynamic_bitset speaks all of it and is still refused.
        static_assert(not wrappable<std::bitset<64>>);
        static_assert(not wrappable<boost::dynamic_bitset<>>);
        static_assert(not wrappable<std::vector<bool>>);
        static_assert(not wrappable<std::vector<std::uint8_t>>);
}

// The public name is built on the wrapper over a packed array, with the word type in the open.
BOOST_AUTO_TEST_CASE(TheBitsetIsTheWrapperOverAPackedArray)
{
        static_assert(std::derived_from<xstd::basic_bitset<std::uint8_t, 9>, xstd::bits::detail::bitset_adaptor<xstd::bits::detail::contiguous_bit_array<std::uint8_t, 9>, xstd::basic_bitset<std::uint8_t, 9>>>);
        static_assert(std::derived_from<xstd::bitset<64>, xstd::bits::detail::bitset_adaptor<xstd::bits::detail::contiguous_bit_array<std::size_t, 64>, xstd::bitset<64>>>);
}

using Static = std::tuple<xstd::basic_bitset<std::uint8_t, 0>, xstd::basic_bitset<std::uint8_t, 1>, xstd::basic_bitset<std::uint8_t, 64>, xstd::basic_bitset<std::uint8_t, 65>, xstd::basic_bitset<std::uint8_t, 128>, xstd::bitset<0>, xstd::bitset<64>, xstd::bitset<65>>;

// A regular, nothrow, trivially copyable type that is not a range, which is what std::bitset is.
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

        w.set(3);
        s.set(3);
        w.set(8, true);
        s.set(8, true);
        BOOST_CHECK_EQUAL(w.count(), s.count());
        BOOST_CHECK_EQUAL(w.test(3), s.test(3));
        BOOST_CHECK_EQUAL(w[8], s[8]);
        BOOST_CHECK_EQUAL(w.to_string(), s.to_string());

        w.flip(3);
        s.flip(3);
        w.reset(8);
        s.reset(8);
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

        w.set();
        s.set();
        w <<= 4;
        s <<= 4;
        BOOST_CHECK_EQUAL(w.to_string(), s.to_string());
        w >>= 9;
        s >>= 9;
        BOOST_CHECK(w.none() and s.none());
        w.set(0);
        s.set(0);
        w <<= 9;
        s <<= 9;
        BOOST_CHECK(w.none() and s.none());

        w.flip();
        s.flip();
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

// The forward pair, over the positions a step past the width would otherwise read beyond.
BOOST_AUTO_TEST_CASE(TheForwardSearchesAreTotalPastTheWidth)
{
        auto const d = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(d.find_next(1), 2UZ);
        BOOST_CHECK_EQUAL(d.find_next(8), Ours::npos); // the last position this width has
        BOOST_CHECK_EQUAL(d.find_next(9), Ours::npos); // the first it has not
        BOOST_CHECK_EQUAL(d.find_next(100), Ours::npos);
        BOOST_CHECK_EQUAL(d.find_next(Ours::npos), Ours::npos);
        BOOST_CHECK_EQUAL(Ours().find_next(0), Ours::npos);

        // And at more than one block, where the last block is the one a step past the width would read beyond.
        using Wide = xstd::basic_bitset<std::uint8_t, 70>;
        auto w = Wide();
        w.set(69);
        BOOST_CHECK_EQUAL(w.find_next(68), 69UZ);
        BOOST_CHECK_EQUAL(w.find_next(69), Wide::npos);
        BOOST_CHECK_EQUAL(w.find_next(70), Wide::npos);
        BOOST_CHECK_EQUAL(w.find_next(Wide::npos), Wide::npos);
}

// The reverse pair mirrors boost's forward pair: the highest set position below pos, npos where none.
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
        w.set(1);
        w.set(8);
        w.set(9);
        w.set(69);
        auto forward = std::vector<std::size_t>();
        for (auto i = w.find_first(); i != Wide::npos; i = w.find_next(i)) {
                forward.push_back(i);
        }
        auto backward = std::vector<std::size_t>();
        for (auto i = w.find_last(); i != Wide::npos; i = w.find_prev(i)) {
                backward.push_back(i);
        }
        BOOST_CHECK((forward == std::vector<std::size_t>{1, 8, 9, 69}));
        BOOST_CHECK(std::ranges::equal(forward, std::views::reverse(backward)));
}

// The three walks over one bitset answer the same positions in the same order, the scan included.
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

        // One position at a time, the last included: that is the step whose next entry is past the width.
        for (auto const i : std::views::iota(0UZ, all.size())) {
                auto one = T();
                one.set(i);
                BOOST_CHECK(walks_agree(one));
        }

        // A stride leaving whole blocks empty between set positions, where the scan walks blocks not bits.
        auto sparse = T();
        for (auto i = 0UZ; i < sparse.size(); i += 17UZ) {
                sparse.set(i);
        }
        BOOST_CHECK(walks_agree(sparse));
}

// The same at a run-time width, whose scan takes the general arm rather than a one- or two-block unrolling.
BOOST_AUTO_TEST_CASE(TheScanWalksWhatIterationWalksAtARunTimeWidth)
{
        for (auto const n : {0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 129UZ, 512UZ}) {
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

// LWG 4294's Constraints, plus one clause of ours: a pointer to a block is the block range's argument.
BOOST_AUTO_TEST_CASE(TheStringConstructorTakesAnyCharLikeTypeButABlock)
{
        static_assert(std::is_constructible_v<Ours, char const*>);
        static_assert(std::is_constructible_v<Ours, wchar_t const*>);
        static_assert(std::is_constructible_v<Ours, char8_t const*>);
        static_assert(std::is_constructible_v<Ours, char16_t const*>);
        static_assert(std::is_constructible_v<Ours, char32_t const*>);

        // Called, and not merely asked about: is_constructible_v never instantiates the body.
        BOOST_CHECK(Ours("101") == Ours("101"));
        BOOST_CHECK(Ours(L"101") == Ours("101"));
        BOOST_CHECK(Ours(u8"101") == Ours("101"));
        BOOST_CHECK(Ours(u"101") == Ours("101"));
        BOOST_CHECK(Ours(U"101") == Ours("101"));

        BOOST_CHECK_THROW(static_cast<void>(Ours("102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(L"102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(u8"102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(u"102")), std::invalid_argument);
        BOOST_CHECK_THROW(static_cast<void>(Ours(U"102")), std::invalid_argument);

        // The widening: not one of the five, and constructible all the same.
        static_assert(std::is_constructible_v<Ours, digit_char const*>);
        static_assert(std::is_constructible_v<std::bitset<9>, digit_char const*>);

        constexpr auto zero = digit_char{static_cast<unsigned char>('0')};
        constexpr auto one = digit_char{static_cast<unsigned char>('1')};
        auto const text = std::array<digit_char, 4>{one, zero, one, digit_char{0}};
        BOOST_CHECK(Ours(text.data(), std::basic_string_view<digit_char>::npos, zero, one) == Ours("101"));

        // And its error path, the third arm: char-like, and neither a character nor a number to a narrow format.
        auto const bad = std::array<digit_char, 4>{one, digit_char{static_cast<unsigned char>('2')}, one, digit_char{0}};
        BOOST_CHECK_THROW(static_cast<void>(Ours(bad.data(), std::basic_string_view<digit_char>::npos, zero, one)), std::invalid_argument);

        // The one subtraction, and the reason for it: the block-range constructor keeps its argument.
        static_assert(std::same_as<Ours::block_type, std::uint8_t>);
        static_assert(not std::is_constructible_v<Ours, std::uint8_t const*>);
}

// boost's block interface: the block type and width, the count, every block out and at most every block in.
BOOST_AUTO_TEST_CASE(TheBlockInterfaceIsBoosts)
{
        static_assert(std::same_as<Ours::block_type, std::uint8_t>);
        static_assert(Ours::bits_per_block == 8UZ);
        BOOST_CHECK_EQUAL(Ours().num_blocks(), 2UZ);

        auto const b = Ours("101000001");
        auto out = std::vector<std::uint8_t>();
        to_block_range(b, std::back_inserter(out));
        BOOST_CHECK((out == std::vector<std::uint8_t>{0b0100'0001, 0b1}));

        auto c = Ours();
        from_block_range(out.begin(), out.end(), c);
        BOOST_CHECK(c == b);

        // Fewer blocks than there are leaves the rest alone; a dirty tail is masked rather than kept.
        auto const dirty = std::array<std::uint8_t, 2>{0b1000'0000, 0b1111'1111};
        auto e = Ours();
        from_block_range(dirty.begin(), dirty.end(), e);
        BOOST_CHECK_EQUAL(e.count(), 2UZ);
        BOOST_CHECK(e.test(7) and e.test(8));
        from_block_range(dirty.begin(), dirty.begin() + 1, c);
        BOOST_CHECK_EQUAL(c.count(), 2UZ);
        BOOST_CHECK(c.test(7) and c.test(8));
}

// from_block_range copies where its source is contiguous and sized, and loops where it is not: both arms.
BOOST_AUTO_TEST_CASE(TheTwoArmsOfFromBlockRangeAgree)
{
        // A list is neither contiguous nor sized, so it takes the loop where a vector takes the copy.
        static_assert(not std::contiguous_iterator<std::list<std::uint8_t>::const_iterator>);
        static_assert(std::contiguous_iterator<std::vector<std::uint8_t>::const_iterator>);

        auto const sources = std::vector<std::vector<std::uint8_t>>{
                {},            // nothing named leaves the target alone
                {0b1010'0101}, // fewer blocks than there are
                {0b1010'0101, 0b1},
                {0b0000'0000, 0b0},
                {0b1111'1111, 0b1111'1111}, // a dirty tail, masked by both arms alike
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

// The same question for the way out: nine bits over eight-bit blocks, so the last block is mostly tail.
BOOST_AUTO_TEST_CASE(TheTwoArmsOfToBlockRangeAgree)
{
        static_assert(std::contiguous_iterator<std::vector<std::uint8_t>::iterator>);
        static_assert(not std::contiguous_iterator<std::back_insert_iterator<std::vector<std::uint8_t>>>);
        static_assert(not std::contiguous_iterator<std::list<std::uint8_t>::iterator>);

        auto const patterns = std::vector<std::string>{
                "000000000",
                "111111111",
                "101000001",
                "100000000",
                "000000001",
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

// boost's remaining members at a static width, where every guard throws; no allocator, the storage having none.
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

        // pos + len is the sum the guard refuses to make: near the top of size_t it wraps below every width.
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
        a.set(1);
        a.set(69);
        b.set(1);
        b.set(2);

        // Named rather than called on the temporaries: clang 23 crashes on a deducing-this call on a prvalue.
        auto const va = xstd::bit_set_view(a);
        auto const vb = xstd::bit_set_view(b);
        auto const qa = xstd::bit_span(a);

        auto keys = std::vector<std::size_t>();
        for (auto const k : va) {
                keys.push_back(k);
        }
        BOOST_CHECK((keys == std::vector<std::size_t>{1, 69}));

        BOOST_CHECK(va != vb);
        BOOST_CHECK(std::is_lt(vb <=> va));
        BOOST_CHECK(va.is_subset_of(va));
        BOOST_CHECK_EQUAL(qa[69], true);

        static_assert(std::same_as<decltype(va), xstd::bit_set_view<std::array<std::uint8_t, 9>, 70> const>);
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::as_const(a))), xstd::bit_set_view<std::array<std::uint8_t, 9> const, 70>>);
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

// [bitset.cons]/2 and [bitset.members]/34-37: the word in and out, and the overflow beyond it.
BOOST_AUTO_TEST_CASE(TheWordConstructorAndConversionsAgreeWithStdBitset)
{
        auto const s = std::bitset<9>(0b101ULL);
        auto const p = Ours(0b101ULL);
        BOOST_CHECK_EQUAL(p.to_string(), s.to_string());
        BOOST_CHECK_EQUAL(p.to_ullong(), s.to_ullong());
        BOOST_CHECK_EQUAL(p.to_ulong(), s.to_ulong());

        // The high bits of the value drop where the width is narrower, as [bitset.cons]/2 has it.
        using Narrow = xstd::basic_bitset<std::uint8_t, 3>;
        using Empty = xstd::basic_bitset<std::uint8_t, 0>;
        BOOST_CHECK_EQUAL(Narrow(0b1111ULL).to_ullong(), 7ULL);
        BOOST_CHECK_EQUAL(Narrow(0b1101ULL).to_ullong(), 5ULL);
        BOOST_CHECK_EQUAL(Empty(0b1111ULL).to_ullong(), 0ULL);

        auto wide = xstd::basic_bitset<std::uint8_t, 70>();
        BOOST_CHECK_EQUAL(wide.to_ullong(), 0ULL);
        wide.set(69);
        BOOST_CHECK_THROW(static_cast<void>(wide.to_ullong()), std::overflow_error);
        BOOST_CHECK_THROW(static_cast<void>(wide.to_ulong()), std::overflow_error);
}

// [bitset.operators]/6: a short read lands in the low bits, and an empty read fails the stream.
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

        // A view over a bitset binds the storage it wraps, the bitset itself being what the constraint refuses.
        using Blocks = std::array<std::size_t, 2>;
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::declval<B&>())), xstd::bit_set_view<Blocks, 100>>);
        static_assert(std::same_as<decltype(xstd::bit_span(std::declval<B&>())), xstd::bit_span<Blocks, 100>>);

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

// The byte exchange at the bitset reading, on container_source rather than on bit_castable.
BOOST_AUTO_TEST_CASE(ABitsetReadingExchangesBytesWithAnotherFieldOfBits)
{
        constexpr auto N = 100UZ;
        auto src = std::bitset<N>();
        for (auto i = 0UZ; i < N; i += 3UZ) {
                src.set(i);
        }

        auto const b = xstd::bit_cast<xstd::bitset<N>>(src);
        BOOST_CHECK_EQUAL(b.count(), src.count());
        for (auto const i : std::views::iota(0UZ, N)) {
                BOOST_CHECK_EQUAL(b.test(i), src.test(i));
        }
        BOOST_CHECK(b.to_bits<std::bitset<N>>() == src);

        static_assert([] -> bool {
                auto const bs = std::bitset<64>(0x0F1E'2D3C'4B5A'6978ULL);
                return xstd::bit_cast<xstd::bitset<64>>(bs).to_bits<std::bitset<64>>() == bs;
        }());
}

// The integer door is untouched: admitting the integer family would collide rather than widen, in two ways.
BOOST_AUTO_TEST_CASE(TheIntegerDoorIsUnchangedByTheByteExchange)
{
        // [bitset.cons]/2's constructor is implicit, so an explicit template would win and refuse this.
        static_assert(std::is_convertible_v<unsigned long long, xstd::bitset<64>>);
        static_assert(std::is_convertible_v<unsigned, xstd::bitset<32>>);
        xstd::bitset<32> const implicitly = 5U;
        BOOST_CHECK_EQUAL(implicitly.to_ullong(), 5ULL);

        // And to_ullong keeps its contract: a position beyond the word throws where a byte copy keeps the low bits.
        auto wide = xstd::bitset<100>();
        wide.set(99);
        BOOST_CHECK_THROW((void)wide.to_ullong(), std::overflow_error);
        // The byte exchange over the same object does not throw, because it is not the same question.
        BOOST_CHECK(wide.to_bits<std::bitset<100>>().test(99));
}

// Two block widths over the same N are two spellings of one field of bits: the byte is the common ground.
BOOST_AUTO_TEST_CASE(TwoBlockWidthsCrossOnTheSameRule)
{
        static_assert([] -> bool {
                auto const wide = xstd::bit_cast<xstd::basic_bitset<std::uint64_t, 64>>(std::bitset<64>(0xABCDULL));
                auto const narrow = xstd::bit_cast<xstd::basic_bitset<std::uint8_t, 64>>(wide);
                return narrow.to_bits<std::bitset<64>>() == std::bitset<64>(0xABCDULL);
        }());
}

// Named both ways, any other width no exchange at all, and a run-time width none of it.
BOOST_AUTO_TEST_CASE(TheBitsetExchangeIsNamedAndWidthExact)
{
        constexpr auto N = 64UZ;
        using T = xstd::bitset<N>;
        static_assert(test::casts_between<T, std::bitset<N>>);

        // The unnamed doors are closed: a conversion operator would be invisible to the concept above.
        static_assert(not std::is_constructible_v<T, std::bitset<N>>);
        static_assert(not std::is_constructible_v<std::bitset<N>, T>);
        static_assert(not std::is_convertible_v<std::bitset<N>, T>);
        static_assert(not std::is_convertible_v<T, std::bitset<N>>);

        // ITS width, not merely one that fits, in the direction the width is checked.
        static_assert(not test::casts_from<T, std::bitset<N + 1UZ>>);
        static_assert(not test::casts_from<T, std::bitset<N - 1UZ>>);

        using Dynamic = xstd::basic_dynamic_bitset<std::uint64_t>;
        static_assert(not test::casts_from<Dynamic, std::bitset<N>>);
        static_assert(not test::exchanges_to_bits<Dynamic, std::bitset<N>>);
}

// A sequence of blocks is a field of bits, so this reading takes it where it declines the bare scalar.
BOOST_AUTO_TEST_CASE(ASequenceOfBlocksIsAFieldOfBitsAndAScalarIsNot)
{
        using Blocks = std::array<std::uint64_t, 2>;
        static_assert(test::exchanges_bits<xstd::bitset<128>, Blocks>);

        // And a scalar is not, asked of the exchange itself rather than of is_constructible_v.
        static_assert(not test::exchanges_from_bits<xstd::bitset<128>, unsigned long long>);
        static_assert(not test::exchanges_to_bits<xstd::bitset<128>, unsigned long long>);

        // The scalar door stays the standard's, implicit and throwing, rather than a byte copy.
        static_assert(std::is_convertible_v<unsigned long long, xstd::bitset<128>>);

        static_assert([] -> bool {
                auto const b = Blocks{0x0123'4567'89AB'CDEFULL, 0xFEDC'BA98'7654'3210ULL};
                return xstd::bitset<128>(xstd::from_bit_storage, b).to_bits<Blocks>() == b;
        }());
}

BOOST_AUTO_TEST_SUITE_END()

// The common vocabulary is structural: it asks the positional members, which the counterparts answer too.
BOOST_AUTO_TEST_SUITE(TheStructuralQuestionIsNotTheNominalOne)

static_assert(test::bitset::vocabulary<xstd::bits::detail::contiguous_bit_array<std::uint64_t, 64>>);
static_assert(test::bitset::vocabulary<boost::dynamic_bitset<>>);
static_assert(test::bitset::vocabulary<std::bitset<64>>);

BOOST_AUTO_TEST_SUITE_END()
