//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <test/sequence/concepts.hpp>        // bit_sequence
#include <xstd/bits/sequence_adaptor.hpp>  // sequence_adaptor
#include <xstd/bits/bit_vector.hpp>          // bit_vector
#include <xstd/bits/block_sequence.hpp>      // block_vector
#include <xstd/bits/ownership.hpp>           // ownership
#include <xstd/bits/bit_array.hpp>           // basic_bit_array
#include <xstd/bits/bit_span.hpp>            // bit_span
#include <algorithm>                         // copy, equal
#include <concepts>                          // same_as
#include <cstddef>                           // size_t
#include <cstdint>                           // uint8_t
#include <functional>                        // hash
#include <iterator>                          // next
#include <memory>                            // allocator
#include <ranges>                            // equal, from_range, iota, next, transform
#include <type_traits>                       // is_default_constructible_v
#include <utility>                           // move
#include <vector>                            // vector
#include <version>                           // IWYU pragma: keep; __cpp_lib_containers_ranges

BOOST_AUTO_TEST_SUITE(BitVector)

using T = xstd::basic_bit_vector<std::uint8_t>;

// Dependent, so a constrained-away member is a false rather than a hard error.
template<class X>
constexpr bool can_grow = requires (X& x) { x.push_back(true); x.resize(1UZ); };

template<class X>
constexpr bool can_flip = requires (X x) { x.flip(); };

template<class X>
constexpr bool has_range_members = requires (X x, std::vector<bool> const& r) { x.append_range(r); x.insert_range(x.cbegin(), r); x.erase(x.cbegin()); };

// std::vector<bool> under its own name: the sequence adaptor over a heap of blocks. [design.md#the-public-names]
BOOST_AUTO_TEST_CASE(TheDynamicSequenceIsTheSequenceAdaptorOverAHeapOfBlocks)
{
        static_assert(std::same_as<T, xstd::sequence_adaptor<xstd::block_vector<std::uint8_t>, xstd::ownership::owns, false>>);
        static_assert(std::same_as<xstd::basic_bit_vector<std::uint8_t, std::allocator<std::uint8_t>>, T>);
        static_assert(test::sequence::bit_sequence<T>);
}

// [vector]'s constructors, every shape, against std::vector<bool> built the same way.
// [vector.bool]'s synopsis line by line, the model first so the checklist is known to be honest. [design.md#the-sequence-contract]
BOOST_AUTO_TEST_CASE(ItAnswersEveryLineOfStdVectorBool)
{
        static_assert(test::sequence::vector_bool<std::vector<bool>>);
        static_assert(test::sequence::vector_bool<T>);
        static_assert(test::sequence::vector_bool<xstd::bit_vector>);
#ifdef __cpp_lib_containers_ranges
        static_assert(test::sequence::vector_bool_ranges<std::vector<bool>>);
#endif
        static_assert(test::sequence::vector_bool_ranges<T>);
        static_assert(test::sequence::vector_bool_ranges<xstd::bit_vector>);
        static_assert(std::same_as<T::allocator_type, std::allocator<std::uint8_t>>);
}

BOOST_AUTO_TEST_CASE(ItIsBuiltLikeAStdVector)
{
        auto const pattern = std::views::iota(0UZ, 20UZ) | std::views::transform([](auto i) { return i % 3 == 0; });
        auto const model = std::vector<bool>(pattern.begin(), pattern.end());

        BOOST_CHECK(T().empty());
        BOOST_CHECK_EQUAL(T(17).size(), 17UZ);
        BOOST_CHECK(std::ranges::equal(T(5, true), std::vector<bool>(5, true)));
        BOOST_CHECK(std::ranges::equal(T(5, false), std::vector<bool>(5, false)));
        BOOST_CHECK(std::ranges::equal(T(pattern.begin(), pattern.end()), model));
        BOOST_CHECK(std::ranges::equal(T(std::from_range, pattern), model));
        BOOST_CHECK(std::ranges::equal(T{ true, false, true }, std::vector<bool>{ true, false, true }));

        auto v = T();
        v = { false, true };
        BOOST_CHECK(std::ranges::equal(v, std::vector<bool>{ false, true }));
        v.assign(3, true);
        BOOST_CHECK(std::ranges::equal(v, std::vector<bool>(3, true)));
        v.assign(pattern.begin(), pattern.end());
        BOOST_CHECK(std::ranges::equal(v, model));
        v.assign({ true });
        BOOST_CHECK(std::ranges::equal(v, std::vector<bool>{ true }));
}

// The allocator forms, each against the one without: the allocator is a construction argument, never part of the value.
BOOST_AUTO_TEST_CASE(ItIsBuiltWithAnAllocatorLikeAStdVector)
{
        auto const pattern = std::views::iota(0UZ, 20UZ) | std::views::transform([](auto i) { return i % 3 == 0; });
        auto const alloc = std::allocator<std::uint8_t>();
        auto const model = T(pattern.begin(), pattern.end());

        BOOST_CHECK(T(alloc).get_allocator() == alloc);
        BOOST_CHECK(T(alloc).empty());
        BOOST_CHECK(T(17, alloc) == T(17));
        BOOST_CHECK(T(5, true, alloc) == T(5, true));
        BOOST_CHECK(T(5, false, alloc) == T(5, false));
        BOOST_CHECK(T(pattern.begin(), pattern.end(), alloc) == model);
        BOOST_CHECK(T(std::from_range, pattern, alloc) == model);
        BOOST_CHECK(T(model, alloc) == model);
        BOOST_CHECK(T({ true, false, true }, alloc) == T({ true, false, true }));

        auto source = model;
        auto const moved = T(std::move(source), alloc);
        BOOST_CHECK(moved == model);
        BOOST_CHECK(source.empty());  // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved): the moved-from is empty by contract, which is the check.
}

// [vector.erasure] against the model, erasing a value and then a predicate.
BOOST_AUTO_TEST_CASE(ErasureIsTheStdVectorsOwn)
{
        auto v = T{ true, false, true, true, false, false, true };
        auto m = std::vector<bool>{ true, false, true, true, false, false, true };
        BOOST_CHECK_EQUAL(erase(v, true), std::erase(m, true));
        BOOST_CHECK(std::ranges::equal(v, m));
        BOOST_CHECK_EQUAL(erase_if(v, [](bool x) -> bool { return not x; }), std::erase_if(m, [](bool x) -> bool { return not x; }));
        BOOST_CHECK(v.empty());
        BOOST_CHECK_EQUAL(erase(v, false), 0UZ);
}

// Growth is the owner's: push, pop, emplace, resize, reserve, shrink and clear, each against the model.
BOOST_AUTO_TEST_CASE(ItGrowsLikeAStdVector)
{
        auto v = T();
        auto m = std::vector<bool>();
        for (auto i = 0UZ; i < 30UZ; ++i) {
                v.push_back(i % 2 == 0);
                m.push_back(i % 2 == 0);
        }
        BOOST_CHECK(std::ranges::equal(v, m));
        BOOST_CHECK_EQUAL(static_cast<bool>(v.emplace_back(true)), true);
        v.pop_back();
        BOOST_CHECK(std::ranges::equal(v, m));

        v.resize(40, true);
        m.resize(40, true);
        BOOST_CHECK(std::ranges::equal(v, m));
        v.resize(7);
        m.resize(7);
        BOOST_CHECK(std::ranges::equal(v, m));

        v.reserve(100);
        BOOST_CHECK_GE(v.capacity(), 100UZ);
        BOOST_CHECK(std::ranges::equal(v, m));
        v.shrink_to_fit();
        BOOST_CHECK_GE(v.capacity(), v.size());

        BOOST_CHECK_EQUAL(v.max_size(), xstd::block_vector<std::uint8_t>().max_size());

        v.clear();
        BOOST_CHECK(v.empty());
        BOOST_CHECK_EQUAL(v.size(), 0UZ);
}

namespace {

// A std::vector<bool> holding what a sequence holds, the model every check below compares against.
template<class R>
auto model_of(R const& r)
        -> std::vector<bool>
{
        return std::vector<bool>(r.begin(), r.end());
}

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
        for (auto i = 0UZ; i < n; ++i) {
                v[i] = (i % 3 == 0) or (i % 7 == 1);
        }
        return v;
}

}       // namespace

// append_range's first tier: another sequence read by block, at every alignment the source and the destination can have. [design.md#the-blit]
BOOST_AUTO_TEST_CASE(AppendRangeBlitsFromASequenceAtAnyAlignment)
{
        auto const source = T(std::from_range, pattern(50));
        auto const view = xstd::bit_span(source);

        for (auto const start : { 0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 16UZ, 40UZ, 43UZ }) {
                for (auto const count : { 0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, 50UZ - start }) {
                        if (start + count > 50UZ) {
                                continue;
                        }
                        for (auto const prefix : { 0UZ, 1UZ, 8UZ, 11UZ }) {
                                auto v = T(std::from_range, pattern(prefix));
                                auto m = model_of(v);
                                auto const window = view.subspan(start, count);
                                v.append_range(window);
                                append_to(m, model_of(window));
                                BOOST_CHECK(std::ranges::equal(v, m));
                                BOOST_CHECK_EQUAL(v.size(), prefix + count);
                        }
                }
        }

        // An owner, a whole view and a static array all blit alike; a source of another block type packs instead.
        auto fixed = xstd::basic_bit_array<9, std::uint8_t>();
        std::ranges::copy(pattern(9), fixed.begin());
        auto v = T();
        v.append_range(source);
        v.append_range(view);
        v.append_range(fixed);
        v.append_range(xstd::basic_bit_vector<std::uint64_t>(std::from_range, pattern(13)));
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

// append_range's second tier: any range of bools, packed a word at a time, the last word trimmed. [design.md#the-blit]
BOOST_AUTO_TEST_CASE(AppendRangePacksAnyRangeOfBools)
{
        for (auto const prefix : { 0UZ, 3UZ, 8UZ }) {
                for (auto const count : { 0UZ, 1UZ, 8UZ, 9UZ, 16UZ, 23UZ }) {
                        auto v = T(std::from_range, pattern(prefix));
                        auto m = model_of(v);
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

        auto v = T{ true };
        v.assign_range(pattern(20));
        BOOST_CHECK(std::ranges::equal(v, pattern(20)));
}

namespace {

// The position under test on the sequence and on the model alike.
template<class C>
auto at(C const& c, std::size_t pos)
{
        return std::ranges::next(c.cbegin(), static_cast<std::ptrdiff_t>(pos));
}

// The two results first, their offsets after: begin() is taken once the insert has moved everything.
template<class V, class M>
auto same_offset_and_contents(V const& v, typename V::iterator vit, M const& m, typename M::iterator mit)
{
        BOOST_CHECK_EQUAL(vit - v.begin(), mit - m.begin());
        BOOST_CHECK(std::ranges::equal(v, m));
}

}       // namespace

// insert's single-value shapes and emplace, rebuilt around the position, against the model. [design.md#the-range-members]
BOOST_AUTO_TEST_CASE(InsertingValuesRebuildsAsAStdVectorDoes)
{
        for (auto const pos : { 0UZ, 1UZ, 8UZ, 13UZ, 20UZ }) {
                auto v = T(std::from_range, pattern(20));
                auto m = pattern(20);

                same_offset_and_contents(v, v.insert(at(v, pos), true), m, m.insert(at(m, pos), true));
                same_offset_and_contents(v, v.insert(at(v, pos), 3UZ, false), m, m.insert(at(m, pos), 3UZ, false));
                same_offset_and_contents(v, v.emplace(at(v, pos), false), m, m.emplace(at(m, pos), false));
        }
}

// insert's range shapes and insert_range, one of them a window into the sequence itself.
BOOST_AUTO_TEST_CASE(InsertingRangesRebuildsAsAStdVectorDoes)
{
        for (auto const pos : { 0UZ, 1UZ, 8UZ, 13UZ, 20UZ }) {
                auto v = T(std::from_range, pattern(20));
                auto m = pattern(20);

                // A range shorter than a word, one of exactly a word, and one that spills into a second: the packing tier's three endings.
                auto const more = pattern(11);
                same_offset_and_contents(v, v.insert(at(v, pos), more.begin(), more.end()), m, m.insert(at(m, pos), more.begin(), more.end()));
                auto const word = pattern(8);
                same_offset_and_contents(v, v.insert(at(v, pos), word.begin(), word.end()), m, m.insert(at(m, pos), word.begin(), word.end()));
                same_offset_and_contents(v, v.insert(at(v, pos), { true, true, false }), m, m.insert(at(m, pos), { true, true, false }));
                same_offset_and_contents(v, v.insert(at(v, pos), { true, false, true, false, true, false, true, false }), m, m.insert(at(m, pos), { true, false, true, false, true, false, true, false }));
                same_offset_and_contents(v, v.insert(at(v, pos), { true, false, true, false, true, false, true, false, true }), m, m.insert(at(m, pos), { true, false, true, false, true, false, true, false, true }));
                auto const middle = std::vector<bool>(m.begin() + 2, m.begin() + 11);
                same_offset_and_contents(v, v.insert_range(at(v, pos), xstd::bit_span(v).subspan(2, 9)), m, m.insert(at(m, pos), middle.begin(), middle.end()));
        }
}

// erase in both shapes, an empty range included.
BOOST_AUTO_TEST_CASE(ErasingRebuildsAsAStdVectorDoes)
{
        for (auto const pos : { 0UZ, 1UZ, 8UZ, 13UZ, 19UZ }) {
                auto v = T(std::from_range, pattern(40));
                auto m = pattern(40);

                same_offset_and_contents(v, v.erase(at(v, pos)), m, m.erase(at(m, pos)));
                same_offset_and_contents(v, v.erase(at(v, pos), at(v, pos + 9)), m, m.erase(at(m, pos), at(m, pos + 9)));
                same_offset_and_contents(v, v.erase(at(v, pos), at(v, pos)), m, m.erase(at(m, pos), at(m, pos)));
        }
}

// [vector.bool]'s two: flip every bit, and swap two proxies.
BOOST_AUTO_TEST_CASE(FlipAndSwapAreStdVectorBools)
{
        auto v = T(std::from_range, pattern(20));
        auto m = pattern(20);
        v.flip();
        m.flip();
        BOOST_CHECK(std::ranges::equal(v, m));

        // Ours is [vector.bool]'s static swap, which the clause still has; the model's own is deprecated by C++26
        // (LWG-3638, P3612R1) and MSVC 2026 says so under /WX, so the model's two bits are exchanged directly.
        T::swap(v[0], v[1]);
        auto const m0 = static_cast<bool>(m[0]);
        m[0] = static_cast<bool>(m[1]);
        m[1] = m0;
        BOOST_CHECK(std::ranges::equal(v, m));

        // A view flips what it views, a window does not. [design.md#windows]
        xstd::bit_span(v).flip();
        m.flip();
        BOOST_CHECK(std::ranges::equal(v, m));
        static_assert(not can_flip<decltype(xstd::bit_span(v).first(2))>);
        static_assert(    can_flip<decltype(xstd::bit_span(v))>);
}

// The owner hashes as std::vector<bool> does, equal values equal; the view over it no more than std::span does. [design.md#the-hashing-invariant]
BOOST_AUTO_TEST_CASE(TheOwnerHashesAndTheViewDoesNot)
{
        auto const h = std::hash<T>();
        BOOST_CHECK_EQUAL(h(T({ true, false, true })), h(T({ true, false, true })));
        BOOST_CHECK(h(T({ true, false, true })) != h(T({ true, false, true, false })));
        BOOST_CHECK(h(T()) != h(T(1)));
        static_assert(not std::is_default_constructible_v<std::hash<xstd::bit_span<xstd::block_vector<std::uint8_t>>>>);
}

// The view over it refers into the block_vector and cannot grow it. [design.md#views-over-owners]
BOOST_AUTO_TEST_CASE(AViewOverItCannotGrowIt)
{
        auto v = T(5);
        auto const s = xstd::bit_span(v);
        s[2] = true;
        BOOST_CHECK(static_cast<bool>(v[2]));

        static_assert(not can_grow<decltype(s)>);
        static_assert(    can_grow<T>);
        static_assert(not has_range_members<decltype(s)>);
        static_assert(    has_range_members<T>);
}

BOOST_AUTO_TEST_SUITE_END()
