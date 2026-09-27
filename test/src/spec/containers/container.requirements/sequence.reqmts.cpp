//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // L1, all_width_position_counts, all_width_positions, all_widths
#include <test/sequence/factory.hpp>    // limit_v, model_of
#include <test/sequence/primitives.hpp> // alternating, constructor, mem_append_range, mem_assign, mem_assign_range, mem_at, mem_back, mem_clear, mem_emplace, mem_emplace_back, mem_erase, mem_front, mem_insert, mem_insert_range, mem_pop_back, mem_push_back, mem_subscript, op_assign, unsized_alternating
#include <test/spec/random.hpp>         // all_sequence_key_pairs, all_sequences
#include <test/spec/sequence.hpp>       // boundary_widths, growable_boundary_widths, growable_every_width, growable_few_widths, growable_random_widths, random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <algorithm>                    // min
#include <array>                        // array
#include <cstddef>                      // size_t
#include <ranges>                       // from_range, iota

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(SequenceReqmts)

using namespace test::sequence;

namespace {

// The lengths a range can take in: none, one, a byte either side, and past two bytes.
constexpr auto lengths = std::array{0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ};

// Up to 33 positions from a random position on, and no more than the sequence holds past it.
[[nodiscard]] auto span_from(auto const& a, std::size_t p)
        -> std::size_t
{
        return std::ranges::min(a.size() - p, 33UZ);
}

// The first p positions of a random sequence, which leaves room for what is inserted at a capacity.
template<class X>
[[nodiscard]] auto prefix(X const& a, std::size_t p)
        -> X
{
        return X(a.begin(), a.begin() + static_cast<X::difference_type>(p));
}

} // namespace

// [sequence.reqmts]/5-7: X u(n, t)
BOOST_AUTO_TEST_SUITE(CountConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        for (auto const n : std::views::iota(0UZ, limit_v<T, L1> + 1UZ)) {
                constructor<T>()(n, false);
                constructor<T>()(n, true);
        }
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/8-10: X u(i, j)
BOOST_AUTO_TEST_SUITE(IteratorConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        for (auto const n : std::views::iota(0UZ, limit_v<T, L1> + 1UZ)) {
                auto const sized = alternating(n);
                auto unsized = unsized_alternating(n);
                constructor<T>()(sized.begin(), sized.end());
                constructor<T>()(unsized.begin(), unsized.end());
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                auto const m = model_of(a);
                constructor<T>()(m.begin(), m.end());
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/11-14: X(from_range, rg)
BOOST_AUTO_TEST_SUITE(RangeConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        for (auto const n : std::views::iota(0UZ, limit_v<T, L1> + 1UZ)) {
                constructor<T>()(std::from_range, alternating(n));
                constructor<T>()(std::from_range, unsized_alternating(n));
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                constructor<T>()(std::from_range, model_of(a));
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/15: X(il)
BOOST_AUTO_TEST_SUITE(InitializerListConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsForShortLists, T, test::spec::sequence::growable_every_width)
{
        constructor<T>()({});
        constructor<T>()({true});
        constructor<T>()({true, false, true});
        constructor<T>()({true, false, true, false, true, false, true, false});
        constructor<T>()({true, false, true, false, true, false, true, false, true});
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/16-19: a = il
BOOST_AUTO_TEST_SUITE(InitializerListAssignment)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                op_assign()(a, {});
                op_assign()(a, {false, true});
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/20-23: a.emplace(p, args)
BOOST_AUTO_TEST_SUITE(Emplace)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndPosition, T, test::spec::sequence::growable_boundary_widths)
{
        on2::all_width_positions<T>([](auto const& a, std::size_t p) {
                mem_emplace()(a, p, false);
                mem_emplace()(a, p, true);
                mem_emplace()(a, p);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                mem_emplace()(prefix(a, p), p / 2UZ, true);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/24-31: a.insert(p, t) and a.insert(p, rv)
BOOST_AUTO_TEST_SUITE(Insert)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndPosition, T, test::spec::sequence::growable_boundary_widths)
{
        on2::all_width_positions<T>([](auto const& a, std::size_t p) {
                mem_insert()(a, p, false);
                mem_insert()(a, p, true);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                mem_insert()(prefix(a, p), p / 2UZ, false);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/32-35: a.insert(p, n, t)
BOOST_AUTO_TEST_SUITE(InsertCount)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthPositionAndCount, T, test::spec::sequence::growable_few_widths)
{
        on3::all_width_position_counts<T>([](auto const& a, std::size_t p, std::size_t k) {
                mem_insert()(a, p, k, true);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                mem_insert()(prefix(a, p), p / 2UZ, span_from(a, p), true);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/36-39: a.insert(p, i, j)
BOOST_AUTO_TEST_SUITE(InsertIterators)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthPositionAndCount, T, test::spec::sequence::growable_few_widths)
{
        on3::all_width_position_counts<T>([](auto const& a, std::size_t p, std::size_t k) {
                auto const sized = alternating(k);
                auto unsized = unsized_alternating(k);
                mem_insert()(a, p, sized.begin(), sized.end());
                mem_insert()(a, p, unsized.begin(), unsized.end());
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                auto const in = alternating(span_from(a, p));
                mem_insert()(prefix(a, p), p / 2UZ, in.begin(), in.end());
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/40-43: a.insert_range(p, rg)
BOOST_AUTO_TEST_SUITE(InsertRange)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthPositionAndCount, T, test::spec::sequence::growable_few_widths)
{
        on3::all_width_position_counts<T>([](auto const& a, std::size_t p, std::size_t k) {
                mem_insert_range()(a, p, alternating(k));
                mem_insert_range()(a, p, unsized_alternating(k));
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                mem_insert_range()(prefix(a, p), p / 2UZ, alternating(span_from(a, p)));
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/44: a.insert(p, il)
BOOST_AUTO_TEST_SUITE(InsertInitializerList)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndPosition, T, test::spec::sequence::growable_boundary_widths)
{
        on2::all_width_positions<T>([](auto const& a, std::size_t p) {
                mem_insert()(a, p, {true, false, true});
                mem_insert()(a, p, {true, false, true, false, true, false, true, false});
                mem_insert()(a, p, {true, false, true, false, true, false, true, false, true});
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/45-48: a.erase(q)
BOOST_AUTO_TEST_SUITE(Erase)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndPosition, T, test::spec::sequence::growable_boundary_widths)
{
        on2::all_width_positions<T>(mem_erase());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>(mem_erase());
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/49-52: a.erase(q1, q2)
BOOST_AUTO_TEST_SUITE(EraseRange)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthPositionAndCount, T, test::spec::sequence::growable_few_widths)
{
        on3::all_width_position_counts<T>([](auto const& a, std::size_t p, std::size_t k) {
                mem_erase()(a, p, p + k);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                mem_erase()(a, p, p + span_from(a, p));
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/53-56: a.clear()
BOOST_AUTO_TEST_SUITE(Clear)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>(mem_clear());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequences<T>(mem_clear());
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/57-59: a.assign(i, j)
BOOST_AUTO_TEST_SUITE(AssignIterators)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndLength, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                for (auto const k : lengths) {
                        auto const sized = alternating(k);
                        auto unsized = unsized_alternating(k);
                        mem_assign()(a, sized.begin(), sized.end());
                        mem_assign()(a, unsized.begin(), unsized.end());
                }
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                auto const in = alternating(a.size());
                mem_assign()(a, in.begin(), in.end());
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/60-64: a.assign_range(rg)
BOOST_AUTO_TEST_SUITE(AssignRange)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndLength, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                for (auto const k : lengths) {
                        mem_assign_range()(a, alternating(k));
                        mem_assign_range()(a, unsized_alternating(k));
                }
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                mem_assign_range()(a, alternating(a.size()));
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/65: a.assign(il)
BOOST_AUTO_TEST_SUITE(AssignInitializerList)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                mem_assign()(a, {});
                mem_assign()(a, {true, false, true});
                mem_assign()(a, {true, false, true, false, true, false, true, false});
                mem_assign()(a, {true, false, true, false, true, false, true, false, true});
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/66-68: a.assign(n, t)
BOOST_AUTO_TEST_SUITE(AssignCount)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndLength, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                for (auto const k : lengths) {
                        mem_assign()(a, k, false);
                        mem_assign()(a, k, true);
                }
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                mem_assign()(a, a.size(), true);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/71-74: a.front()
BOOST_AUTO_TEST_SUITE(Front)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::boundary_widths)
{
        on1::all_widths<T>(mem_front());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::random_widths)
{
        test::spec::random::all_sequences<T>(mem_front());
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/75-78: a.back()
BOOST_AUTO_TEST_SUITE(Back)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::boundary_widths)
{
        on1::all_widths<T>(mem_back());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::random_widths)
{
        test::spec::random::all_sequences<T>(mem_back());
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/84-88: a.emplace_back(args)
BOOST_AUTO_TEST_SUITE(EmplaceBack)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                mem_emplace_back()(a, false);
                mem_emplace_back()(a, true);
                mem_emplace_back()(a);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomPrefixes, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                mem_emplace_back()(prefix(a, p), true);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/101-108: a.push_back(t) and a.push_back(rv)
BOOST_AUTO_TEST_SUITE(PushBack)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                mem_push_back()(a, false);
                mem_push_back()(a, true);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomPrefixes, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                mem_push_back()(prefix(a, p), false);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/109-112: a.append_range(rg)
BOOST_AUTO_TEST_SUITE(AppendRange)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndLength, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                for (auto const k : lengths) {
                        mem_append_range()(a, alternating(k));
                        mem_append_range()(a, unsized_alternating(k));
                }
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t p) {
                mem_append_range()(prefix(a, p), alternating(span_from(a, p)));
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/117-120: a.pop_back()
BOOST_AUTO_TEST_SUITE(PopBack)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::growable_boundary_widths)
{
        on1::all_widths<T>(mem_pop_back());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::growable_random_widths)
{
        test::spec::random::all_sequences<T>(mem_pop_back());
}

BOOST_AUTO_TEST_SUITE_END()

// [sequence.reqmts]/121-128: a[n] and a.at(n)
BOOST_AUTO_TEST_SUITE(Subscript)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndPosition, T, test::spec::sequence::boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                mem_subscript()(a);
                for (auto const n : std::views::iota(0UZ, a.size())) {
                        mem_subscript()(a, n);
                        mem_at()(a, n);
                }
                mem_at()(a, a.size());
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                mem_subscript()(a);
                mem_at()(a, a.size());
        });
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t n) {
                mem_subscript()(a, n);
                mem_at()(a, n);
        });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
