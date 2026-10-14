//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>              // bools_of, for_each_sequence, for_each_window, offsets, patterned, sequences, striped_vector
#include <test/for_each_type.hpp>                    // for_each_type
#include <xstd/bits/bit_algorithm/bit_transform.hpp> // bit_transform
#include <xstd/bits/bit_array.hpp>                   // basic_bit_array, bit_array
#include <xstd/bits/bit_span.hpp>                    // bit_span
#include <xstd/bits/bit_vector.hpp>                  // bit_vector
#include <boost/test/unit_test.hpp>                  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                 // binary_transform_result, transform, unary_transform_result
#include <concepts>                                  // same_as
#include <cstddef>                                   // ptrdiff_t, size_t
#include <cstdint>                                   // uint64_t, uint8_t
#include <functional>                                // bit_and, bit_or, bit_xor, equal_to, greater_equal, identity, less, logical_and, logical_not, logical_or, not_equal_to
#include <ranges>                                    // begin, dangling, end, iterator_t
#include <string>                                    // string
#include <tuple>                                     // apply, tuple
#include <utility>                                   // declval
#include <vector>                                    // vector

BOOST_AUTO_TEST_SUITE(BitTransform)

namespace {

template<class R, class O, class F>
concept transformable = requires (R const& r, O out, F f) { xstd::bit_transform(r, out, f); };

// Every function of one bool, each spelled as a caller would.
constexpr auto unary_functions = std::tuple([](bool) -> bool { return false; }, [](bool) -> bool { return true; }, std::identity(), std::logical_not<>(), [](bool b) -> int { return b ? 0 : 7; });

// The four set operations' truth tables and more, as functors and as a lambda.
constexpr auto binary_functions = std::tuple(std::bit_and<>(), std::bit_or<>(), std::bit_xor<>(), std::logical_and<>(), std::logical_or<>(), std::equal_to<>(), std::not_equal_to<>(), std::less<>(), std::greater_equal<>(), [](bool x, bool y) -> bool { return x and not y; });

// The algorithm and std::ranges::transform from r into a striped destination over Block at an offset.
template<class Block, class R, class F>
[[nodiscard]] auto agrees(R const& r, std::size_t offset, F f)
        -> bool
{
        auto const source = test::algorithm::bools_of(r);
        auto dst          = test::algorithm::striped_vector<Block>(source.size() + offset + 5UZ);
        auto model        = test::algorithm::bools_of(dst);
        auto const d      = static_cast<std::ptrdiff_t>(offset);
        auto const want   = std::ranges::transform(source, model.begin() + d, f);
        auto const got    = xstd::bit_transform(r, dst.begin() + d, f);
        return got.in == std::ranges::end(r) and got.out - dst.begin() == want.out - model.begin() and test::algorithm::bools_of(dst) == model;
}

// The binary form between r and a striped second operand over Block, into a third at an offset, as far as the shorter.
template<class Block, class R, class F>
[[nodiscard]] auto agrees2(R const& r, std::size_t length, std::size_t offset, F f)
        -> bool
{
        auto const y    = test::algorithm::striped_vector<Block>(length);
        auto const xm   = test::algorithm::bools_of(r);
        auto const ym   = test::algorithm::bools_of(y);
        auto dst        = test::algorithm::striped_vector<std::uint8_t>(xm.size() + offset + 5UZ);
        auto model      = test::algorithm::bools_of(dst);
        auto const d    = static_cast<std::ptrdiff_t>(offset);
        auto const want = std::ranges::transform(xm, ym, model.begin() + d, f);
        auto const got  = xstd::bit_transform(r, y, dst.begin() + d, f);
        return got.in1 - std::ranges::begin(r) == want.in1 - xm.begin() and got.in2 - y.begin() == want.in2 - ym.begin() and got.out - dst.begin() == want.out - model.begin() and test::algorithm::bools_of(dst) == model;
}

// Every function of one bool, into either block width at every offset.
template<class R>
[[nodiscard]] auto unary_disagreements(R const& r)
        -> int
{
        auto wrong = 0;
        for (auto const offset : test::algorithm::offsets) {
                std::apply([&](auto... f) -> void { wrong += ((static_cast<int>(not agrees<std::uint8_t>(r, offset, f)) + static_cast<int>(not agrees<std::uint64_t>(r, offset, f))) + ...); }, unary_functions);
        }
        return wrong;
}

// Every binary function, against a shorter, an equal and a longer second operand of either block width.
template<class R>
[[nodiscard]] auto binary_disagreements(R const& r)
        -> int
{
        auto const n = r.size();
        auto wrong   = 0;
        for (auto const length : {n / 2UZ, n, n + 9UZ}) {
                std::apply([&](auto... f) -> void { wrong += ((static_cast<int>(not agrees2<std::uint8_t>(r, length, 3UZ, f)) + static_cast<int>(not agrees2<std::uint64_t>(r, length, 64UZ, f))) + ...); }, binary_functions);
        }
        return wrong;
}

} // namespace

// Every input of every column of ours through every function of one bool.
BOOST_AUTO_TEST_CASE(AgreesWithRangesTransformOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += unary_disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// Every input of every column of ours through every function of two bools.
BOOST_AUTO_TEST_CASE(TheBinaryFormAgreesWithRangesTransformOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += binary_disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window transforms its own positions alone, at every offset and length, in either form.
BOOST_AUTO_TEST_CASE(AWindowTransformsItsOwnPositions)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += unary_disagreements(w) + binary_disagreements(w); });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// In place, out being the first input's begin, as std::ranges::transform allows.
BOOST_AUTO_TEST_CASE(TransformsInPlace)
{
        auto v     = test::algorithm::patterned(xstd::bit_vector(130UZ));
        auto model = test::algorithm::bools_of(v);
        std::ranges::transform(model, model.begin(), std::logical_not<>());
        static_cast<void>(xstd::bit_transform(v, v.begin(), std::logical_not<>()));
        BOOST_CHECK(test::algorithm::bools_of(v) == model);
        auto const w = test::algorithm::patterned(xstd::bit_vector(70UZ));
        std::ranges::transform(model, test::algorithm::bools_of(w), model.begin(), std::bit_xor<>());
        static_cast<void>(xstd::bit_transform(v, w, v.begin(), std::bit_xor<>()));
        BOOST_CHECK(test::algorithm::bools_of(v) == model);
}

// In a constant expression, between bools of ours alone, with a function that answers a bool.
BOOST_AUTO_TEST_CASE(TransformsBetweenOurSequencesAlone)
{
        static_assert([] -> bool {
                auto a       = test::algorithm::patterned(xstd::bit_array<70>());
                auto const r = xstd::bit_transform(a, a, a.begin(), std::bit_xor<>());
                return r.out == a.end() and test::algorithm::bools_of(a) == test::algorithm::bools_of(xstd::bit_array<70>());
        }());
        using A   = xstd::bit_array<8>;
        using It  = std::ranges::iterator_t<A>;
        using Not = std::logical_not<>;
        static_assert(transformable<A, It, Not> and not transformable<A, std::ranges::iterator_t<A const>, Not> and not transformable<std::vector<bool>, It, Not>);
        static_assert(not transformable<A, It, decltype([](bool) -> std::string { return {}; })> and not transformable<A, It, decltype([](bool&) -> bool { return true; })>);
        static_assert(std::same_as<decltype(xstd::bit_transform(A(), std::declval<It>(), Not())), std::ranges::unary_transform_result<std::ranges::dangling, It>>);
        static_assert(std::same_as<decltype(xstd::bit_transform(A(), std::declval<A&>(), std::declval<It>(), std::bit_and<>())), std::ranges::binary_transform_result<std::ranges::dangling, It, It>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
