//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/allocator.hpp>               // basic_guarantee, copy_and_swap_propagating, copy_propagating, ledger_allocator, strong_guarantee
#include <test/sanitizer.hpp>                         // IWYU pragma: keep; TEST_HAS_ADDRESS_SANITIZER
#include <test/sequence/dense.hpp>                    // yields_every_position
#include <test/sequence/rotation.hpp>                 // permutation_sweep, permutes_ten_bits
#include <test/whole_blocks.hpp>                      // block_bits
#include <xstd/bits/bit_algorithm/bit_count.hpp>      // bit_count
#include <xstd/bits/bit_algorithm/bit_reverse.hpp>    // bit_reverse
#include <xstd/bits/bit_algorithm/bit_rotate.hpp>     // bit_rotate
#include <xstd/bits/bit_array.hpp>                    // basic_bit_array
#include <xstd/bits/bit_span/bit_span.hpp>            // bit_span
#include <xstd/bits/bit_vector.hpp>                   // bit_vector
#include <xstd/bits/detail/bit_block_container.hpp>   // bit_block_container
#include <xstd/bits/detail/ownership.hpp>             // storage
#include <xstd/bits/detail/sequence_adaptor.hpp>      // sequence_adaptor
#include <boost/iterator/function_input_iterator.hpp> // make_function_input_iterator
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_LT, BOOST_CHECK_THROW
#include <algorithm>                                  // copy, equal, is_sorted, ranges::count, ranges::is_sorted, ranges::sort, sort
#include <array>                                      // array
#include <concepts>                                   // same_as
#include <cstddef>                                    // ptrdiff_t, size_t
#include <cstdint>                                    // int64_t, uint64_t, uint8_t
#include <functional>                                 // ranges::greater
#include <iterator>                                   // sentinel_for
#include <limits>                                     // numeric_limits
#include <memory>                                     // allocator
#include <new>                                        // IWYU pragma: keep; bad_alloc, named only without TEST_HAS_ADDRESS_SANITIZER
#include <ranges>                                     // equal, from_range, iota, to, transform
#include <stdexcept>                                  // length_error, runtime_error
#include <type_traits>                                // is_convertible_v, is_nothrow_convertible_v
#include <utility>                                    // declval, move
#include <vector>                                     // vector
#include <version>                                    // IWYU pragma: keep; __cpp_lib_containers_ranges

BOOST_AUTO_TEST_SUITE(BitVector)

using T = xstd::basic_bit_vector<std::vector<std::uint8_t>>;

// Dependent, so a constrained-away member is a false rather than a hard error.
template<class X>
constexpr bool can_grow = requires (X& x) { x.push_back(true); x.resize(1UZ); };

template<class X>
constexpr bool can_flip = requires (X x) { x.flip(); };

template<class X>
constexpr bool has_static_swap = requires (X x) { X::swap(x[0], x[0]); };

template<class X>
constexpr bool can_permute = requires (X x) { xstd::ranges::bit_rotate(x, x.begin()); xstd::ranges::bit_reverse(x); };

template<class X>
constexpr bool has_range_members = requires (X x, std::vector<bool> const& r) { x.append_range(r); x.insert_range(x.cbegin(), r); x.erase(x.cbegin()); };

// std::vector<bool> under its own name: the sequence adaptor over a heap of blocks.
BOOST_AUTO_TEST_CASE(TheDynamicSequenceIsTheSequenceAdaptorOverAHeapOfBlocks)
{
        static_assert(std::derived_from<T, xstd::bits::detail::sequence_adaptor<xstd::bits::detail::bit_block_container<std::vector<std::uint8_t>>, xstd::bits::detail::storage::owned, xstd::bits::detail::window::all, T>>);
        static_assert(std::same_as<xstd::basic_bit_vector<std::vector<std::uint8_t, std::allocator<std::uint8_t>>>, T>);
        static_assert(std::same_as<T::allocator_type, std::allocator<std::uint8_t>>);
}

// The allocator forms, each against the one without: the allocator is a construction argument, never part of the value.
BOOST_AUTO_TEST_CASE(ItIsBuiltWithAnAllocatorLikeAStdVector)
{
        auto const pattern = std::views::iota(0UZ, 20UZ) | std::views::transform([](auto i) { return i % 3 == 0; });
        auto const alloc   = std::allocator<std::uint8_t>();
        auto const model   = T(pattern.begin(), pattern.end());

        BOOST_CHECK(T(alloc).get_allocator() == alloc);
        BOOST_CHECK(T(alloc).empty());
        BOOST_CHECK(T(17, alloc) == T(17));
        BOOST_CHECK(T(5, true, alloc) == T(5, true));
        BOOST_CHECK(T(5, false, alloc) == T(5, false));
        BOOST_CHECK(T(pattern.begin(), pattern.end(), alloc) == model);
        BOOST_CHECK(T(std::from_range, pattern, alloc) == model);
        BOOST_CHECK(T(model, alloc) == model);
        BOOST_CHECK(T({true, false, true}, alloc) == T({true, false, true}));

        auto source      = model;
        auto const moved = T(std::move(source), alloc);
        BOOST_CHECK(moved == model);
        BOOST_CHECK(source.empty()); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved): the moved-from is empty by contract, which is the check.
}

namespace {

// A wide source copied in as an lvalue, on x's ledger under an allocator that differs from x's.
template<class X>
[[nodiscard]] auto copy_in_from(X const& t)
{
        return [&t](X& x) -> void {
                auto const source = X(t, test::container::ledger_allocator<X>(*x.get_allocator().book(), 1));
                x                 = source;
        };
}

} // namespace

// An allocator a copy hands over and a move does not: taken strongly where a swap hands it over, else basically.
BOOST_AUTO_TEST_CASE(ACopyTakesAnAllocatorThatAMoveCannotAsStronglyAsASwapAllows)
{
        using swapping           = xstd::basic_bit_vector<std::vector<std::uint8_t, test::container::copy_and_swap_propagating<std::uint8_t>>>;
        auto const wide_swapping = swapping(1000UZ, true, swapping::allocator_type(1));
        auto narrow_swapping     = swapping(1UZ, true, swapping::allocator_type(0));
        narrow_swapping          = wide_swapping;
        BOOST_CHECK(narrow_swapping == wide_swapping and narrow_swapping.get_allocator() == wide_swapping.get_allocator());
        auto alike_swapping = swapping(1UZ, true, swapping::allocator_type(1));
        alike_swapping      = wide_swapping;
        BOOST_CHECK(alike_swapping == wide_swapping);
        BOOST_CHECK(test::container::strong_guarantee(swapping(1UZ, true), copy_in_from(wide_swapping)));

        using copying           = xstd::basic_bit_vector<std::vector<std::uint8_t, test::container::copy_propagating<std::uint8_t>>>;
        auto const wide_copying = copying(1000UZ, true, copying::allocator_type(1));
        auto narrow_copying     = copying(1UZ, true, copying::allocator_type(0));
        narrow_copying          = wide_copying;
        BOOST_CHECK(narrow_copying == wide_copying and narrow_copying.get_allocator() == wide_copying.get_allocator());
        auto alike_copying = copying(1UZ, true, copying::allocator_type(1));
        alike_copying      = wide_copying;
        BOOST_CHECK(alike_copying == wide_copying);
        BOOST_CHECK(test::container::basic_guarantee(copying(1UZ, true), copy_in_from(wide_copying)));
}

// A std::vector<bool>'s ceiling is what a distance can name, where the storage's own bound is whole blocks.
BOOST_AUTO_TEST_CASE(TheCeilingIsWhatADistanceCanName)
{
        auto v = T(7);
        BOOST_CHECK_EQUAL(v.max_size(), xstd::bits::detail::bit_block_container<std::vector<std::uint8_t>>::max_addressable_width);
        BOOST_CHECK_LT(v.max_size(), xstd::bits::detail::bit_block_container<std::vector<std::uint8_t>>().max_size());
        BOOST_CHECK_LE(v.max_size(), static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()));
        BOOST_CHECK_EQUAL(v.max_size() % 8UZ, 0UZ);
        BOOST_CHECK_THROW(v.resize(v.max_size() + 1UZ), std::length_error);
        BOOST_CHECK_EQUAL(v.size(), 7UZ);
}

namespace {

// A std::vector<bool> holding what a sequence holds, the model every check below compares against.
template<class R>
auto model_of(R const& r)
        -> std::vector<bool>
{
        return std::vector<bool>(r.begin(), r.end());
}

// The ceiling row for row, over blocks as wide as a std::vector<bool>'s word so the two are comparable at all.
BOOST_AUTO_TEST_CASE(TheCeilingIsStdVectorBoolsRowForRow)
{
        constexpr auto pmax = static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max());
        auto v              = xstd::bit_vector();
        auto m              = std::vector<bool>();

        // Both bound by what a distance can name, and ours never above theirs.
        BOOST_CHECK_LE(v.max_size(), pmax);
        BOOST_CHECK_LE(m.max_size(), pmax);
        BOOST_CHECK_LE(v.max_size(), m.max_size());
        BOOST_CHECK_EQUAL(v.max_size() % test::block_bits, 0UZ);

        // One past each bound is std::length_error on both, and neither asks for memory first.
        BOOST_CHECK_THROW(v.resize(v.max_size() + 1UZ), std::length_error);
        BOOST_CHECK_THROW(m.resize(m.max_size() + 1UZ), std::length_error);

        // And one past what any distance can name, where a std::vector<bool> answers std::length_error.
        BOOST_CHECK_THROW(v.resize(pmax + 1UZ), std::length_error);
        BOOST_CHECK_THROW(m.resize(pmax + 1UZ), std::length_error);

        // Nothing moved: a refused growth is not a partial one.
        BOOST_CHECK(v.empty());
        BOOST_CHECK(m.empty());
}

#ifndef TEST_HAS_ADDRESS_SANITIZER

// The last row asks for the memory, so the allocator answers; ours only, the two libraries not agreeing here.
BOOST_AUTO_TEST_CASE(TheLastRowIsTheAllocatorsToRefuse)
{
        // A 32-bit size_t makes that row a few hundred megabytes, which an allocator may well grant.
        if constexpr (test::block_bits >= 64UZ) {
                auto v = xstd::bit_vector();
                BOOST_CHECK_THROW(v.resize(v.max_size()), std::bad_alloc);
                BOOST_CHECK(v.empty());
        }
}

#endif

// std::vector<bool>'s append_range and insert_range, spelled through insert for the standard libraries that lack them.
template<class R>
auto append_to(std::vector<bool>& m, R const& r)
        -> void
{
        m.insert(m.end(), r.begin(), r.end());
}

// A pattern over n positions with a period that never aligns with a block.
auto pattern(std::size_t n)
        -> std::vector<bool>
{
        auto v = std::vector<bool>(n);
        for (auto const i : std::views::iota(0UZ, n)) {
                v[i] = (i % 3 == 0) or (i % 7 == 1);
        }
        return v;
}

} // namespace

// append_range's first tier: another sequence read by block, at every alignment source and destination can have.
BOOST_AUTO_TEST_CASE(AppendRangeBlitsFromASequenceAtAnyAlignment)
{
        auto const source = T(std::from_range, pattern(50));
        auto const view   = xstd::bit_span(source);

        for (auto const start : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 16UZ, 40UZ, 43UZ}) {
                for (auto const count : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, 50UZ - start}) {
                        if (start + count > 50UZ) {
                                continue;
                        }
                        for (auto const prefix : {0UZ, 1UZ, 8UZ, 11UZ}) {
                                auto v            = T(std::from_range, pattern(prefix));
                                auto m            = model_of(v);
                                auto const window = view.subspan(start, count);
                                v.append_range(window);
                                append_to(m, model_of(window));
                                BOOST_CHECK(std::ranges::equal(v, m));
                                BOOST_CHECK_EQUAL(v.size(), prefix + count);
                        }
                }
        }

        // An owner, a whole view and a static array all blit alike; a source of another block type packs instead.
        auto fixed = xstd::basic_bit_array<std::uint8_t, 9>();
        std::ranges::copy(pattern(9), fixed.begin());
        auto v = T();
        v.append_range(source);
        v.append_range(view);
        v.append_range(fixed);
        v.append_range(xstd::basic_bit_vector<std::vector<std::uint64_t>>(std::from_range, pattern(13)));
        auto m = std::vector<bool>();
        append_to(m, pattern(50));
        append_to(m, pattern(50));
        append_to(m, pattern(9));
        append_to(m, pattern(13));
        BOOST_CHECK(std::ranges::equal(v, m));

        // Itself, through a view: the blit reads only below the old width, which no append touches.
        auto w = T(std::from_range, pattern(21));
        w.append_range(xstd::bit_span(w).subspan(3, 15));
        auto n = pattern(21);
        append_to(n, std::vector<bool>(n.begin() + 3, n.begin() + 18));
        BOOST_CHECK(std::ranges::equal(w, n));
}

// append_range's second tier: any range of bools, packed a block at a time, the last block trimmed.
BOOST_AUTO_TEST_CASE(AppendRangePacksAnyRangeOfBools)
{
        for (auto const prefix : {0UZ, 3UZ, 8UZ}) {
                for (auto const count : {0UZ, 1UZ, 8UZ, 9UZ, 16UZ, 23UZ}) {
                        auto v          = T(std::from_range, pattern(prefix));
                        auto m          = model_of(v);
                        auto const more = pattern(count);
                        v.append_range(more);
                        append_to(m, more);
                        BOOST_CHECK(std::ranges::equal(v, m));

                        // An input range with no size to reserve.
                        auto const lazy = std::views::iota(0UZ, count) | std::views::transform([](auto i) { return i % 2 == 1; });
                        v.append_range(lazy);
                        append_to(m, lazy);
                        BOOST_CHECK(std::ranges::equal(v, m));
                }
        }

        auto v = T{true};
        v.assign_range(pattern(20));
        BOOST_CHECK(std::ranges::equal(v, pattern(20)));
}

// insert_range from a window into the sequence itself, with room to spare or none: it rebuilds, reading the old width.
BOOST_AUTO_TEST_CASE(InsertingAWindowOfItselfRebuildsAsAStdVectorDoes)
{
        for (auto const room : {0UZ, 200UZ}) {
                for (auto const pos : {0UZ, 1UZ, 8UZ, 13UZ, 20UZ}) {
                        auto v = T(std::from_range, pattern(20));
                        v.reserve(room);
                        auto m            = pattern(20);
                        auto const middle = std::vector<bool>(m.begin() + 2, m.begin() + 11);
                        auto const r      = v.insert_range(v.cbegin() + static_cast<std::ptrdiff_t>(pos), xstd::bit_span(v).subspan(2, 9));
                        m.insert(m.cbegin() + static_cast<std::ptrdiff_t>(pos), middle.begin(), middle.end());
                        BOOST_CHECK_EQUAL(r - v.begin(), static_cast<std::ptrdiff_t>(pos));
                        BOOST_CHECK(std::ranges::equal(v, m));
                }
        }
}

// Within capacity every shape of insertion shifts the tail in place, so with each allocation refused, all complete.
BOOST_AUTO_TEST_CASE(InsertingWithinCapacityAllocatesNothing)
{
        using X           = xstd::basic_bit_vector<std::vector<std::uint8_t, test::container::propagating<std::uint8_t>>>;
        auto const blocks = T(std::from_range, pattern(13));
        auto const bools  = pattern(11);
        for (auto const pos : {0UZ, 1UZ, 8UZ, 13UZ, 20UZ}) {
                auto book = test::container::ledger();
                auto v    = X(std::from_range, pattern(20), test::container::ledger_allocator<X>(book));
                v.reserve(100);
                auto m        = pattern(20);
                auto const at = static_cast<std::ptrdiff_t>(pos);
                book.budget   = 0;
                v.insert(v.cbegin() + at, true);
                m.insert(m.cbegin() + at, true);
                v.insert(v.cbegin() + at, 9UZ, false);
                m.insert(m.cbegin() + at, 9UZ, false);
                v.insert_range(v.cbegin() + at, bools);
                m.insert(m.cbegin() + at, bools.begin(), bools.end());
                v.insert_range(v.cbegin() + at, blocks);
                m.insert(m.cbegin() + at, blocks.begin(), blocks.end());
                v.emplace(v.cbegin() + at, true);
                m.insert(m.cbegin() + at, true);
                book.budget = -1;
                BOOST_CHECK_EQUAL(book.refusals, 0);
                BOOST_CHECK(std::ranges::equal(v, m));
        }
}

// Inserting at the end grows as push_back does, geometrically, so n insertions make O(log n) allocations.
BOOST_AUTO_TEST_CASE(InsertingAtTheEndGrowsGeometrically)
{
        using X                = xstd::basic_bit_vector<std::vector<std::uint8_t, test::container::propagating<std::uint8_t>>>;
        constexpr auto n       = 10'000UZ;
        constexpr auto granted = std::int64_t{1'000'000};
        auto book              = test::container::ledger();
        auto v                 = X(test::container::ledger_allocator<X>(book));
        book.budget            = granted;
        for (auto const i : std::views::iota(0UZ, n)) {
                v.insert(v.cend(), i % 3 == 0);
        }
        auto const allocations = granted - book.budget;
        book.budget            = -1;
        BOOST_CHECK_EQUAL(v.size(), n);
        BOOST_CHECK_EQUAL(std::ranges::count(v, true), static_cast<std::ptrdiff_t>((n + 2UZ) / 3UZ));
        BOOST_CHECK_LT(allocations, 32);
}

// A sized append grows through the storage's own resize, as push_back does: n nine-bool appends, O(log n) allocations.
BOOST_AUTO_TEST_CASE(AppendingSmallRangesGrowsGeometrically)
{
        using X                = xstd::basic_bit_vector<std::vector<std::uint8_t, test::container::propagating<std::uint8_t>>>;
        constexpr auto n       = 10'000UZ;
        constexpr auto granted = std::int64_t{1'000'000};
        auto const one         = xstd::bit_vector(1UZ, true);
        auto book              = test::container::ledger();
        auto packed            = X(test::container::ledger_allocator<X>(book));
        auto blitted           = X(test::container::ledger_allocator<X>(book));
        book.budget            = granted;
        for (auto const i : std::views::iota(0UZ, n)) {
                packed.append_range(std::array{i % 3 == 0, false, false, false, false, false, false, false, false});
        }
        auto const packing = granted - book.budget;
        for ([[maybe_unused]] auto const i : std::views::iota(0UZ, n)) {
                blitted.append_range(one);
        }
        auto const blitting = granted - packing - book.budget;
        book.budget         = -1;
        BOOST_CHECK_EQUAL(std::ranges::count(packed, true), static_cast<std::ptrdiff_t>((n + 2UZ) / 3UZ));
        BOOST_CHECK_EQUAL(std::ranges::count(blitted, true), static_cast<std::ptrdiff_t>(n));
        BOOST_CHECK_LT(packing, 32);
        BOOST_CHECK_LT(blitting, 32);
}

// A range that throws while it is read leaves the sequence as it was, the gap opened for it closed again.
BOOST_AUTO_TEST_CASE(InsertingARangeThatThrowsLeavesTheSequenceAsItWas)
{
        // One range type, read whole below the limit and failing part way at it.
        auto const unreadable_at = [](std::size_t limit) -> auto {
                return std::views::iota(0UZ, 11UZ) | std::views::transform([limit](std::size_t i) -> bool {
                               if (i == limit) {
                                       throw std::runtime_error("unreadable");
                               }
                               return i % 2UZ == 0UZ;
                       });
        };
        auto const more = unreadable_at(11UZ) | std::ranges::to<std::vector>();
        for (auto const pos : {0UZ, 1UZ, 8UZ, 13UZ, 20UZ}) {
                auto v        = T(std::from_range, pattern(20));
                auto m        = pattern(20);
                auto const at = static_cast<std::ptrdiff_t>(pos);
                v.insert_range(v.cbegin() + at, unreadable_at(11UZ));
                m.insert(m.cbegin() + at, more.begin(), more.end());
                BOOST_CHECK(std::ranges::equal(v, m));
                auto const before = v;
                BOOST_CHECK_THROW(v.insert_range(v.cbegin() + at, unreadable_at(5UZ)), std::runtime_error);
                BOOST_CHECK(v == before);
        }
}

// Two proxies swap through their own swap, found by ADL; neither the owner nor a view has a static swap or a flip.
BOOST_AUTO_TEST_CASE(TheProxiesSwapAndNothingFlips)
{
        auto v = T(std::from_range, pattern(20));
        auto m = pattern(20);

        swap(v[0], v[1]);
        auto const m0 = static_cast<bool>(m[0]);
        m[0]          = static_cast<bool>(m[1]);
        m[1]          = m0;
        BOOST_CHECK(std::ranges::equal(v, m));
        static_assert(not has_static_swap<T>);
        static_assert(not can_flip<T>);
        static_assert(not can_flip<decltype(xstd::bit_span(v))>);
        static_assert(not can_flip<decltype(xstd::bit_span(v).first(2))>);
}

// vector<bool>::const_reference is bool, so a reference initializes one; the const proxy reads through the same bit.
BOOST_AUTO_TEST_CASE(AReferenceConvertsToAConstReference)
{
        auto v                     = T{true, false};
        T::const_reference const r = v[0];
        BOOST_CHECK(r);
        v[0] = false;
        BOOST_CHECK(not r);
        static_assert(std::is_nothrow_convertible_v<T::reference, T::const_reference>);
        static_assert(not std::is_convertible_v<T::const_reference, T::reference>);
}

// The view over it refers into the owner's std::vector of blocks and cannot grow it.
BOOST_AUTO_TEST_CASE(AViewOverItCannotGrowIt)
{
        auto v       = T(5);
        auto const s = xstd::bit_span(v);
        s[2]         = true;
        BOOST_CHECK(static_cast<bool>(v[2]));

        static_assert(not can_grow<decltype(s)>);
        static_assert(can_grow<T>);
        static_assert(not has_range_members<decltype(s)>);
        static_assert(has_range_members<T>);
}

// Every position, densely, agreeing with the subscript -- and not a contiguous range, which no proxy sequence can be.
BOOST_AUTO_TEST_CASE(ItYieldsEveryPosition)
{
        auto c = T(70);
        test::sequence::yields_every_position(c);

        for (auto const n : std::views::iota(0UZ, c.size())) {
                c[n] = (n % 3UZ == 0UZ);
        }
        test::sequence::yields_every_position(c);
}

// std::sort over the proxies orders random bits as it orders std::vector<bool>'s, both ways, across block boundaries.
BOOST_AUTO_TEST_CASE(SortingRandomBitsLeavesThemSorted)
{
        // Fixed width, not ULL: a fixed seed should reproduce the same sequence on every platform.
        auto lcg            = std::uint64_t{0x9E3779B97F4A7C15};
        auto const next_bit = [&lcg] -> bool { lcg = (lcg * 6364136223846793005ULL) + 1442695040888963407ULL; return (lcg >> 33U & 1U) != 0U; };
        auto const fill     = [&next_bit](T& v, std::vector<bool>& m) -> void {
                for (auto const i : std::views::iota(0UZ, v.size())) {
                        auto const b = next_bit();
                        v[i]         = b;
                        m[i]         = b;
                }
        };

        for (auto const n : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 64UZ, 65UZ, 199UZ, 1000UZ}) {
                auto v = T(n);
                auto m = std::vector<bool>(n);

                // The pre-ranges algorithms on purpose: they reach the bits through iter_swap and the swap friends.
                fill(v, m);
                auto const ones = static_cast<std::size_t>(std::ranges::count(m, true));
                BOOST_CHECK_EQUAL(std::is_sorted(v.begin(), v.end()), std::ranges::is_sorted(m)); // NOLINT(modernize-use-ranges)
                std::sort(v.begin(), v.end());                                                    // NOLINT(modernize-use-ranges)
                std::ranges::sort(m);
                BOOST_CHECK(std::is_sorted(v.begin(), v.end())); // NOLINT(modernize-use-ranges)
                BOOST_CHECK(std::ranges::equal(v, m));
                BOOST_CHECK_EQUAL(static_cast<std::size_t>(xstd::ranges::bit_count(v, true)), ones);

                // Descending under std::ranges::greater: every true before every false.
                fill(v, m);
                std::ranges::sort(v, std::ranges::greater());
                std::ranges::sort(m, std::ranges::greater());
                BOOST_CHECK(std::ranges::is_sorted(v, std::ranges::greater()));
                BOOST_CHECK(std::ranges::equal(v, m));
        }
}

// std::vector's guides: the block from the allocator where one is given, std::size_t where none is.
BOOST_AUTO_TEST_CASE(ItDeducesAsStdVectorDoes)
{
        auto const bools = std::vector<bool>{true, false, true, true};
        auto const alloc = std::allocator<std::uint8_t>();

        auto const a = xstd::basic_bit_vector(bools.begin(), bools.end());
        static_assert(std::same_as<decltype(a), xstd::bit_vector const>);
        static_assert(std::same_as<decltype(std::vector(bools.begin(), bools.end())), std::vector<bool>>);
        auto const b = xstd::basic_bit_vector(bools.begin(), bools.end(), alloc);
        static_assert(std::same_as<decltype(b), xstd::basic_bit_vector<std::vector<std::uint8_t>> const>);
        auto const c = xstd::basic_bit_vector(std::from_range, bools);
        static_assert(std::same_as<decltype(c), xstd::bit_vector const>);
        auto const d = xstd::basic_bit_vector(std::from_range, bools, alloc);
        static_assert(std::same_as<decltype(d), xstd::basic_bit_vector<std::vector<std::uint8_t>> const>);
#ifdef __cpp_lib_containers_ranges

        static_assert(std::same_as<decltype(std::vector(std::from_range, bools)), std::vector<bool>>);

#endif
        auto const e = xstd::basic_bit_vector(b, alloc);
        static_assert(std::same_as<decltype(e), xstd::basic_bit_vector<std::vector<std::uint8_t>> const>);
        auto const f = xstd::basic_bit_vector({true, false, true, true});
        static_assert(std::same_as<decltype(f), xstd::bit_vector const>);
        static_assert(std::same_as<decltype(std::vector({true, false, true, true})), std::vector<bool>>);
        auto const g = xstd::basic_bit_vector({true, false, true, true}, alloc);
        static_assert(std::same_as<decltype(g), xstd::basic_bit_vector<std::vector<std::uint8_t>> const>);

        BOOST_CHECK(std::ranges::equal(a, bools) and std::ranges::equal(b, bools) and std::ranges::equal(c, bools));
        BOOST_CHECK(std::ranges::equal(d, bools) and e == b and f == a and g == b);
}

namespace {

// Every width through three narrow blocks, then whole and partial ones past two wide blocks, through owner or view.
template<class V, class Through = test::sequence::as_owner>
[[nodiscard]] auto permutation_sweeps(Through through = {})
        -> int
{
        auto disagreements = 0;
        for (auto const n : std::views::iota(0UZ, 18UZ)) {
                disagreements += test::sequence::permutation_sweep(V(n), through);
        }
        for (auto const n : {64UZ, 70UZ, 128UZ, 130UZ}) {
                disagreements += test::sequence::permutation_sweep(V(n), through);
        }
        return disagreements;
}

// The view a test permutes through, named so that each sweep over it is one instantiation.
struct as_span
{
        template<class V>
        [[nodiscard]] auto operator()(V& v) const noexcept
        {
                return xstd::bit_span(v);
        }
};

} // namespace

// bit_rotate and bit_reverse, as std::ranges::rotate and std::ranges::reverse move the bools, at every run-time width.
BOOST_AUTO_TEST_CASE(ItRotatesAndReversesAsTheAlgorithmsDo)
{
        static_assert(noexcept(xstd::ranges::bit_rotate(std::declval<T&>(), std::declval<T&>().begin())) and noexcept(xstd::ranges::bit_reverse(std::declval<T&>())));
        static_assert(test::sequence::permutes_ten_bits(T(10)));
        BOOST_CHECK_EQUAL(permutation_sweeps<T>(), 0);
        BOOST_CHECK_EQUAL(permutation_sweeps<xstd::basic_bit_vector<std::vector<std::uint64_t>>>(), 0);
}

// Boost's function_input_iterator has no default constructor, so no sentinel_for, yet std::vector<bool> takes it.
BOOST_AUTO_TEST_CASE(ItTakesACpp17InputIteratorPairAsStdVectorDoes)
{
        auto every_third = [i = 0] mutable -> bool { return i++ % 3 == 0; };
        auto const first = boost::make_function_input_iterator(every_third, 0);
        auto const last  = boost::make_function_input_iterator(every_third, 6);
        static_assert(not std::sentinel_for<decltype(first), decltype(first)>);

        auto v = xstd::basic_bit_vector(first, last);
        static_assert(std::same_as<decltype(v), xstd::bit_vector>);
        BOOST_CHECK(v == (xstd::bit_vector{true, false, false, true, false, false}));

        auto all_set = [] -> bool { return true; };
        v.assign(boost::make_function_input_iterator(all_set, 0), boost::make_function_input_iterator(all_set, 3));
        BOOST_CHECK(v == (xstd::bit_vector{true, true, true}));

        auto none_set  = [] -> bool { return false; };
        auto const pos = v.insert(v.cbegin() + 1, boost::make_function_input_iterator(none_set, 0), boost::make_function_input_iterator(none_set, 2));
        BOOST_CHECK(pos == v.begin() + 1);
        BOOST_CHECK(v == (xstd::bit_vector{true, false, false, true, true}));

        auto const w = T(boost::make_function_input_iterator(all_set, 0), boost::make_function_input_iterator(all_set, 2), std::allocator<std::uint8_t>());
        BOOST_CHECK(w == (T{true, true}));
}

// A view rotates and reverses what it views, as it flips it; a window does neither.
BOOST_AUTO_TEST_CASE(AViewOverItPermutesItAndAWindowDoesNot)
{
        BOOST_CHECK_EQUAL(permutation_sweeps<T>(as_span()), 0);
        static_assert(can_permute<decltype(xstd::bit_span(std::declval<T&>()))>);
        static_assert(not can_permute<decltype(xstd::bit_span(std::declval<T&>()).first(2))>);
        static_assert(not can_permute<decltype(xstd::bit_span(std::declval<T const&>()))>);
}

BOOST_AUTO_TEST_SUITE_END()
