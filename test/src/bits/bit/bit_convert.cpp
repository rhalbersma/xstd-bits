//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>                   // for_each_type
#include <xstd/bits/bit/bit_convert.hpp>            // adopts_from, bit_constructible_from, bit_convert, bit_convertible_to
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array, bit_array
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set, bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector, bit_bounded_vector
#include <xstd/bits/bit_fixed_set.hpp>              // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set, bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/bit_span.hpp>                   // bit_span
#include <xstd/bits/bit_subspan.hpp>                // bit_subspan
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set, bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector, bit_small_vector
#include <xstd/bits/from_bit_storage.hpp>           // from_bit_storage
#include <xstd/ints/memory.hpp>                     // align_up
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <array>                                    // array
#include <bitset>                                   // bitset
#include <concepts>                                 // constructible_from
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t, uint16_t, uint32_t, uint64_t
#include <limits>                                   // numeric_limits
#include <new>                                      // bad_alloc
#include <ranges>                                   // filter, iota, to
#include <span>                                     // dynamic_extent
#include <stdexcept>                                // overflow_error
#include <tuple>                                    // tuple
#include <utility>                                  // declval, move
#include <vector>                                   // vector

BOOST_AUTO_TEST_SUITE(BitConvert)

namespace {

template<class T>
concept is_set = requires { typename T::key_type; };

// The positions a reading holds: a set's keys, and a sequence's true indices.
template<class T>
[[nodiscard]] constexpr auto positions(T const& x)
        -> std::vector<std::size_t>
{
        if constexpr (is_set<T>) {
                return {x.begin(), x.end()};
        } else {
                return std::views::iota(0UZ, x.size()) | std::views::filter([&](std::size_t i) -> bool { return x[i]; }) | std::ranges::to<std::vector>();
        }
}

// A set built from its keys, a sequence at its width with those positions true.
template<class T>
[[nodiscard]] constexpr auto make(std::size_t width, std::vector<std::size_t> const& keys)
        -> T
{
        if constexpr (is_set<T>) {
                return T(keys.begin(), keys.end());
        } else {
                auto x = [&] -> T {
                        if constexpr (std::constructible_from<T, std::size_t>) {
                                return T(width);
                        } else {
                                return T();
                        }
                }();
                for (auto const k : keys) {
                        x[k] = true;
                }
                return x;
        }
}

// The width a source of 100 positions has: a run-time set's whole blocks, and the 100 itself for everything else.
template<class From>
[[nodiscard]] consteval auto width_of_100()
        -> std::size_t
{
        constexpr auto grows = [] -> bool {
                if constexpr (is_set<From>) {
                        return not From::has_static_width;
                } else {
                        return false;
                }
        }();
        if constexpr (grows) {
                return xstd::align_up(100UZ, static_cast<std::size_t>(std::numeric_limits<typename From::block_container_type::value_type>::digits));
        } else {
                return 100UZ;
        }
}

template<class B>
using sources_of = std::tuple<
        xstd::basic_bit_fixed_set<std::size_t, B, 100>,
        xstd::basic_bit_array<B, 100>,
        xstd::basic_bit_set<std::size_t, B>,
        xstd::basic_bit_vector<B>,
        xstd::basic_bit_bounded_set<std::size_t, B, 128>,
        xstd::basic_bit_bounded_vector<B, 128>,
        xstd::basic_bit_small_set<std::size_t, B, 128>,
        xstd::basic_bit_small_vector<B, 128>>;

using sources = decltype(std::tuple_cat(
        sources_of<std::uint8_t>(), sources_of<std::uint16_t>(), sources_of<std::uint32_t>(), sources_of<std::uint64_t>()
));

template<class B>
using targets_of = std::tuple<
        xstd::basic_bit_set<std::size_t, B>,
        xstd::basic_bit_vector<B>,
        xstd::basic_bit_bounded_set<std::size_t, B, 128>,
        xstd::basic_bit_bounded_vector<B, 128>,
        xstd::basic_bit_small_set<std::size_t, B, 128>,
        xstd::basic_bit_small_vector<B, 128>>;

using targets = decltype(std::tuple_cat(targets_of<std::uint8_t>(), targets_of<std::uint64_t>()));

} // namespace

// Every width converts: equal fixed widths, a run-time width into a fixed one, and anything into a run-time one.
BOOST_AUTO_TEST_CASE(BitConvertibleToNamesEveryWidthButAViewTarget)
{
        static_assert(xstd::bit_convertible_to<xstd::bit_fixed_set<64>, std::uint64_t> and xstd::bit_convertible_to<std::bitset<20>, xstd::bit_array<20>>);
        static_assert(xstd::bit_convertible_to<std::array<std::uint8_t, 3>, xstd::bit_fixed_set<24>> and xstd::bit_convertible_to<xstd::bit_array<64>, std::bitset<64>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_set_view<std::uint64_t>, std::uint64_t> and xstd::bit_convertible_to<xstd::bit_span<std::array<std::uint8_t, 3>>, xstd::bit_vector>);
        static_assert(xstd::bit_convertible_to<xstd::bit_set, std::uint64_t> and xstd::bit_convertible_to<xstd::bit_vector, std::bitset<70>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_vector, xstd::bit_array<64>> and xstd::bit_convertible_to<xstd::bit_bounded_set<64>, xstd::bit_fixed_set<10>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_array<64>, xstd::bit_vector> and xstd::bit_convertible_to<std::bitset<70>, xstd::bit_small_vector<64>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_vector, xstd::bit_bounded_set<64>> and xstd::bit_convertible_to<std::uint64_t, xstd::bit_set>);

        // Two fixed widths that differ are no conversion at all, rather than a narrowing or a widening one.
        static_assert(not xstd::bit_convertible_to<std::uint32_t, xstd::bit_array<20>> and not xstd::bit_convertible_to<std::uint64_t, std::bitset<63>>);
        static_assert(not xstd::bit_convertible_to<std::bitset<65>, xstd::bit_array<64>> and not xstd::bit_convertible_to<xstd::bit_fixed_set<64>, xstd::bit_fixed_set<65>>);

        // A view is read from and never written into, and a window's bits are not its storage's.
        static_assert(not xstd::bit_convertible_to<std::uint64_t, xstd::bit_set_view<std::uint64_t>>);
        static_assert(not xstd::bit_convertible_to<xstd::bit_vector, xstd::bit_span<std::array<std::uint8_t, 3>>>);
        static_assert(not xstd::bit_convertible_to<xstd::bit_subspan<std::array<std::uint64_t, 2>, std::dynamic_extent, 100>, xstd::bit_vector>);

        // What has no bit storage converts to nothing.
        static_assert(not xstd::bit_convertible_to<int, xstd::bit_vector> and not xstd::bit_convertible_to<std::vector<bool>, xstd::bit_vector>);
        static_assert(not xstd::bit_convertible_to<xstd::bit_vector, int> and not xstd::bit_convertible_to<xstd::bit_vector, std::vector<std::uint64_t>>);
        BOOST_CHECK(true);
}

// Blocks an owner takes as they are: its own container at a run-time width, or a field at a fixed one.
BOOST_AUTO_TEST_CASE(BitConstructibleFromNamesBlocksTakenAsTheyAre)
{
        static_assert(xstd::bit_constructible_from<xstd::bit_vector, std::vector<std::size_t>> and xstd::bit_constructible_from<xstd::bit_set, std::vector<std::size_t>>);
        static_assert(xstd::bit_constructible_from<xstd::bit_fixed_set<64>, std::array<std::uint64_t, 1>> and xstd::bit_constructible_from<xstd::bit_array<20>, std::array<std::uint8_t, 3>>);
        static_assert(xstd::bit_constructible_from<xstd::bit_bounded_set<100>, xstd::bit_bounded_set<100>::block_container_type>);

        // Another block type, a field too narrow, and a container a bounded owner does not hold are none of those.
        static_assert(not xstd::bit_constructible_from<xstd::bit_vector, std::vector<std::uint8_t>>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_fixed_set<256>, std::array<std::uint64_t, 3>>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_bounded_set<100>, std::array<std::uint64_t, 2>>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_bounded_vector<100>, std::vector<std::size_t>>);

        // What has bit storage without being it, and a view, which owns nothing to take blocks into.
        static_assert(not xstd::bit_constructible_from<xstd::bit_fixed_set<64>, std::bitset<64>>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_vector, xstd::bit_set>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_set_view<std::uint64_t>, std::uint64_t>);
        BOOST_CHECK(true);
}

// Equal fixed widths cross whole, between any two readings and to and from the blocks and a std::bitset.
BOOST_AUTO_TEST_CASE(EqualFixedWidthsCrossWhole)
{
        static_assert([] -> bool {
                auto const set = xstd::bit_fixed_set<64>{0, 5, 63};
                auto const block = xstd::bit_convert<std::uint64_t>(set);
                auto const seq = xstd::bit_convert<xstd::bit_array<64>>(set);
                auto const legacy = xstd::bit_convert<std::bitset<64>>(seq);
                return block == ((1ULL << 63U) | (1ULL << 5U) | 1ULL) and seq[5] and legacy.test(63) and legacy.count() == 3 and xstd::bit_convert<xstd::bit_fixed_set<64>>(legacy) == set;
        }());

        // A width that is no whole number of blocks round-trips through a std::bitset of the same width.
        auto const narrow = xstd::bit_array<20>(xstd::from_bit_storage, std::array<std::uint8_t, 3>{0x01, 0x00, 0x08});
        auto const legacy = xstd::bit_convert<std::bitset<20>>(narrow);
        BOOST_CHECK(legacy.test(0) and legacy.test(19));
        BOOST_CHECK(xstd::bit_convert<xstd::bit_array<20>>(legacy) == narrow);

        // Width zero has no bytes to copy, and still converts.
        BOOST_CHECK(xstd::bit_convert<std::bitset<0>>(xstd::bit_array<0>()).none());

        // A view is read from as the blocks it spans.
        auto board = std::uint64_t{0b1010};
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint64_t>(xstd::bit_set_view(board)), board);
}

// Nothing throws between fixed widths, and only that pair is noexcept: a run-time end can refuse or allocate.
BOOST_AUTO_TEST_CASE(OnlyTwoFixedWidthsAreNoexcept)
{
        static_assert(noexcept(xstd::bit_convert<std::uint64_t>(std::declval<xstd::bit_fixed_set<64> const&>())));
        static_assert(noexcept(xstd::bit_convert<xstd::bit_array<70>>(std::declval<std::bitset<70> const&>())));
        static_assert(not noexcept(xstd::bit_convert<std::uint64_t>(std::declval<xstd::bit_set const&>())));
        static_assert(not noexcept(xstd::bit_convert<xstd::bit_vector>(std::declval<std::bitset<70> const&>())));
        static_assert(not noexcept(xstd::bit_convert<xstd::bit_vector>(std::declval<xstd::bit_set&&>())));
        BOOST_CHECK(true);
}

// Into a fixed width from a run-time one: zero-extended, and std::overflow_error for a position the target cannot hold.
BOOST_AUTO_TEST_CASE(ARunTimeWidthNarrowsByValue)
{
        // A set over a whole 64-bit block converts to ten positions, as long as its keys fit.
        auto const ten = xstd::bit_convert<xstd::bit_fixed_set<10>>(xstd::bit_set{3, 9});
        BOOST_CHECK(positions(ten) == (std::vector<std::size_t>{3, 9}));
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::bit_fixed_set<10>>(xstd::bit_set{3, 10})), std::overflow_error);

        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint64_t>(xstd::bit_set{0, 63}), (1ULL << 63U) | 1ULL);
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<std::uint64_t>(xstd::bit_set{3, 64})), std::overflow_error);

        // A sequence wider than the target narrows when its tail is clear, and a shorter one zero-extends.
        auto wide = xstd::bit_vector(70);
        wide[63] = true;
        BOOST_CHECK(xstd::bit_convert<std::bitset<64>>(wide).test(63));
        wide[69] = true;
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<std::bitset<64>>(wide)), std::overflow_error);
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::bitset<70>>(wide).count(), 2UZ);

        auto const short_one = make<xstd::basic_bit_vector<std::uint8_t>>(3, {2});
        BOOST_CHECK(positions(xstd::bit_convert<xstd::bit_array<64>>(short_one)) == (std::vector<std::size_t>{2}));
        BOOST_CHECK((xstd::bit_convert<std::array<std::uint16_t, 2>>(short_one) == std::array<std::uint16_t, 2>{4, 0}));

        // Width zero holds nothing, so only an empty source fits.
        BOOST_CHECK(xstd::bit_convert<std::bitset<0>>(xstd::bit_set()).none());
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<std::bitset<0>>(xstd::bit_set{0})), std::overflow_error);

        static_assert(xstd::bit_convert<std::uint64_t>(xstd::bit_set{3}) == 8ULL);
}

// Position i is position i, across both readings, every column of source and every block width on either side.
BOOST_AUTO_TEST_CASE(EveryPositionCrossesBetweenReadingsColumnsAndBlockWidths)
{
        auto const keys = std::vector<std::size_t>{0, 1, 7, 8, 15, 16, 31, 32, 63, 64, 99};
        test::for_each_type<sources>([&]<class From> -> void {
                auto const from = make<From>(100, keys);
                test::for_each_type<targets>([&]<class To> -> void {
                        auto const to = xstd::bit_convert<To>(from);
                        BOOST_CHECK(positions(to) == keys);
                        if constexpr (not is_set<To>) {
                                BOOST_CHECK_EQUAL(to.size(), width_of_100<From>());
                        }
                        BOOST_CHECK(positions(xstd::bit_convert<From>(to)) == keys);
                });
        });
}

// A set's universe is its whole blocks: three positions over three 64-bit blocks are a sequence of 192.
BOOST_AUTO_TEST_CASE(ASetSourceGivesASequenceItsWholeBlocks)
{
        auto const v = xstd::bit_convert<xstd::bit_vector>(xstd::bit_set{3, 64, 129});
        BOOST_CHECK_EQUAL(v.size(), 192UZ);
        BOOST_CHECK(positions(v) == (std::vector<std::size_t>{3, 64, 129}));

        // Never past what the set can hold: a fixed or bounded set of 100 is a sequence of 100.
        BOOST_CHECK_EQUAL(xstd::bit_convert<xstd::bit_vector>(xstd::bit_fixed_set<100>{99}).size(), 100UZ);
        BOOST_CHECK_EQUAL(xstd::bit_convert<xstd::bit_vector>(xstd::bit_bounded_set<100>{99}).size(), 100UZ);
}

// A set target covers the source's width in whole blocks of its own, so a sequence of 70 comes back as 72 or 128.
BOOST_AUTO_TEST_CASE(ASetTargetTakesWholeBlocksOfItsOwn)
{
        auto const v = make<xstd::bit_vector>(70, {0, 69});
        BOOST_CHECK_EQUAL(xstd::bit_convert<xstd::bit_vector>(xstd::bit_convert<xstd::basic_bit_set<std::size_t, std::uint8_t>>(v)).size(), 72UZ);
        BOOST_CHECK_EQUAL(xstd::bit_convert<xstd::bit_vector>(xstd::bit_convert<xstd::bit_set>(v)).size(), 128UZ);
}

// The same blocks from an rvalue move over whole: the buffer is the one the source had, and the source is left empty.
BOOST_AUTO_TEST_CASE(AnRvalueOfTheSameBlocksIsAdoptedWithoutACopy)
{
        auto blocks = std::vector<std::size_t>{0b101, 0, 1};
        auto const* const data = blocks.data();
        auto v = xstd::bit_vector(xstd::from_bit_storage, std::move(blocks));
        v.resize(130);

        auto s = xstd::bit_convert<xstd::bit_set>(std::move(v));
        auto const emptied = v.empty(); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move): the moved-from state is the check.
        BOOST_CHECK(emptied);
        BOOST_CHECK(positions(s) == (std::vector<std::size_t>{0, 2, 128}));

        // Adopted whole, a sequence target still takes the source's width: the set's whole blocks.
        auto back = xstd::bit_convert<xstd::bit_vector>(std::move(s));
        BOOST_CHECK(s.empty()); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move): the moved-from state is the check.
        BOOST_CHECK_EQUAL(back.size(), 192UZ);
        BOOST_CHECK(std::move(back).extract().data() == data);

        // A sequence to a sequence keeps its width exactly.
        auto two = std::vector<std::size_t>(2);
        auto const* const wide = two.data();
        auto w = xstd::bit_vector(xstd::from_bit_storage, std::move(two));
        w.resize(70);
        w[69] = true;
        auto same = xstd::bit_convert<xstd::bit_vector>(std::move(w));
        BOOST_CHECK_EQUAL(same.size(), 70UZ);
        BOOST_CHECK(same[69] and not same[68]);
        BOOST_CHECK(std::move(same).extract().data() == wide);
}

// One container type, its allocator with it, and a capacity that is all of it: else the blocks are copied.
BOOST_AUTO_TEST_CASE(OnlyTheSameBlockContainerIsAdopted)
{
        using xstd::bits::detail::adopts_from;
        static_assert(adopts_from<xstd::bit_vector, xstd::bit_set> and adopts_from<xstd::bit_small_set<256>, xstd::bit_small_vector<256>>);
        static_assert(not adopts_from<xstd::bit_vector, xstd::bit_set&> and not adopts_from<xstd::bit_vector, xstd::bit_set const>);
        static_assert(not adopts_from<xstd::bit_vector, xstd::basic_bit_set<std::size_t, std::uint8_t>>);
        static_assert(not adopts_from<xstd::bit_vector, xstd::bit_array<64>>);

        // A bounded capacity short of its blocks' last bit could not hold all of what it adopts.
        static_assert(adopts_from<xstd::bit_bounded_vector<128>, xstd::bit_bounded_set<100>>);
        static_assert(not adopts_from<xstd::bit_bounded_vector<100>, xstd::bit_bounded_set<128>>);

        auto s = xstd::bit_bounded_set<100>{5, 99};
        auto const v = xstd::bit_convert<xstd::bit_bounded_vector<128>>(std::move(s));
        BOOST_CHECK_EQUAL(v.size(), 100UZ);
        BOOST_CHECK(positions(v) == (std::vector<std::size_t>{5, 99}));

        auto small = xstd::bit_small_vector<256>(200);
        small[199] = true;
        auto const t = xstd::bit_convert<xstd::bit_small_set<256>>(std::move(small));
        BOOST_CHECK(positions(t) == (std::vector<std::size_t>{199}));
}

// A copy leaves its source as it was, from an lvalue and from an rvalue whose blocks cannot be adopted alike.
BOOST_AUTO_TEST_CASE(ACopyLeavesTheSourceUnchanged)
{
        auto v = make<xstd::bit_vector>(70, {1, 69});
        auto const before = v;

        auto const s = xstd::bit_convert<xstd::bit_set>(v);
        BOOST_CHECK(v == before);
        BOOST_CHECK(positions(s) == (std::vector<std::size_t>{1, 69}));

        auto const narrow = xstd::bit_convert<xstd::basic_bit_set<std::size_t, std::uint8_t>>(std::move(v));
        BOOST_CHECK(v == before); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move): a copy leaves the source, which is the check.
        BOOST_CHECK(positions(narrow) == (std::vector<std::size_t>{1, 69}));
}

// The copy is shifts in a constant expression, and adoption is a move there as everywhere.
BOOST_AUTO_TEST_CASE(TheConversionIsAConstantExpression)
{
        static_assert([] -> bool {
                auto s = xstd::basic_bit_set<std::size_t, std::uint8_t>{3, 9, 20};
                auto const wide = xstd::bit_convert<xstd::basic_bit_vector<std::uint32_t>>(s);
                auto const adopted = xstd::bit_convert<xstd::basic_bit_vector<std::uint8_t>>(std::move(s));
                return wide.size() == 24UZ and wide[20] and adopted.size() == 24UZ and adopted.count() == 3UZ;
        }());
        static_assert([] -> bool {
                auto a = xstd::basic_bit_array<std::uint16_t, 70>();
                a[0] = true;
                a[69] = true;
                auto const s = xstd::bit_convert<xstd::bit_set>(a);
                auto const v = xstd::bit_convert<xstd::basic_bit_vector<std::uint8_t>>(std::bitset<70>(0b1011));
                return s.contains(69UZ) and s.size() == 2UZ and v.size() == 70UZ and v.count() == 3UZ;
        }());
#ifdef XSTD_BITS_HAS_CONSTEXPR_BOUNDED
        static_assert(xstd::bit_convert<xstd::bit_bounded_vector<100>>(xstd::bit_fixed_set<100>{7, 99}).count() == 2UZ);
#endif
        BOOST_CHECK(true);
}

// A std::bitset is a sequence of its N, at a whole number of bytes and blocks or not, width zero included.
BOOST_AUTO_TEST_CASE(AStdBitsetIsASequenceOfItsWidth)
{
        auto const check = []<std::size_t N> -> void {
                auto b = std::bitset<N>();
                auto keys = std::vector<std::size_t>();
                for (auto i = 0UZ; i < b.size(); i += 7UZ) {
                        b.set(i);
                        keys.push_back(i);
                }
                auto const v = xstd::bit_convert<xstd::bit_vector>(b);
                BOOST_CHECK_EQUAL(v.size(), N);
                BOOST_CHECK(positions(v) == keys);
                BOOST_CHECK(positions(xstd::bit_convert<xstd::basic_bit_set<std::size_t, std::uint8_t>>(b)) == keys);
                BOOST_CHECK(positions(xstd::bit_convert<xstd::basic_bit_vector<std::uint16_t>>(b)) == keys);
                BOOST_CHECK(positions(xstd::bit_convert<xstd::bit_small_set<128>>(b)) == keys);
                BOOST_CHECK(positions(xstd::bit_convert<xstd::bit_bounded_vector<128>>(b)) == keys);
        };
        check.operator()<0>();
        check.operator()<8>();
        check.operator()<64>();
        check.operator()<70>();
        check.operator()<128>();
}

// A bounded target that cannot hold the source throws what its own growth throws, std::bad_alloc.
BOOST_AUTO_TEST_CASE(ABoundedTargetTooSmallThrowsWhatItsGrowthThrows)
{
        // A sequence target takes the source's width, which must fit.
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::bit_bounded_vector<100>>(xstd::bit_vector(101))), std::bad_alloc);
        BOOST_CHECK_EQUAL(xstd::bit_convert<xstd::bit_bounded_vector<100>>(xstd::bit_vector(100)).size(), 100UZ);
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::bit_bounded_vector<100>>(xstd::bit_set{3, 64})), std::bad_alloc);
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::bit_bounded_vector<100>>(std::bitset<128>())), std::bad_alloc);

        // A set target is capped at its capacity, and throws only for a key beyond it.
        BOOST_CHECK(positions(xstd::bit_convert<xstd::bit_bounded_set<100>>(xstd::bit_set{3, 64})) == (std::vector<std::size_t>{3, 64}));
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::bit_bounded_set<100>>(xstd::bit_set{3, 150})), std::bad_alloc);
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 100>>(std::bitset<128>(1ULL << 63U) << 64U)), std::bad_alloc);

        // A wide universe with no key past the capacity fits.
        auto grown = xstd::bit_set{3, 255};
        grown.erase(255);
        BOOST_CHECK_EQUAL(xstd::bit_convert<xstd::bit_vector>(grown).size(), 256UZ);
        BOOST_CHECK(positions(xstd::bit_convert<xstd::bit_bounded_set<100>>(grown)) == (std::vector<std::size_t>{3}));
}

BOOST_AUTO_TEST_SUITE_END()
