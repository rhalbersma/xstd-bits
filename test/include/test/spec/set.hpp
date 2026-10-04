//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_SET_HPP
#define TEST_SPEC_SET_HPP

#include <test/container/allocator.hpp>          // non_propagating
#include <test/flat_set.hpp>                     // IWYU pragma: keep; TEST_HAS_FLAT_SET, flat_set
#include <test/minimal_blocks.hpp>               // minimal_blocks
#include <test/set/exhaustive.hpp>               // L1, L2, L3, L4, limit_v, on0, on1, on2, on3, on4, static_capacity, static_width
#include <test/spec/input.hpp>                   // edge, exhaustive, key_list, key_vector, keyed, listed, memo, one, rebuilt, three, two
#include <test/spec/random.hpp>                  // block_digits_v, key_samples, keyed_samples, pair_samples, triple_samples, width
#include <test/spec/view.hpp>                    // input_t, owner_t, view_traits, view_type, viewed
#include <test/uint128.hpp>                      // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_bounded_set.hpp>         // basic_bit_bounded_set
#include <xstd/bits/bit_fixed_set.hpp>           // basic_bit_fixed_set
#include <xstd/bits/bit_key_traits.hpp>          // bit_key_traits
#include <xstd/bits/bit_set.hpp>                 // basic_bit_set
#include <xstd/bits/bit_set_view.hpp>            // bit_set_view
#include <xstd/bits/detail/bit_container.hpp>    // bit_container
#include <xstd/bits/detail/ownership.hpp>        // owned_storage, storage
#include <xstd/bits/detail/set_adaptor.hpp>      // set_adaptor
#include <xstd/bits/ext/boost/bit_small_set.hpp> // basic_bit_small_set
#include <algorithm>                             // sort
#include <array>                                 // array
#include <cstddef>                               // size_t
#include <cstdint>                               // uint8_t, uint16_t, uint32_t, uint64_t
#include <functional>                            // greater, less
#include <set>                                   // set
#include <span>                                  // dynamic_extent
#include <tuple>                                 // tuple, tuple_cat
#include <utility>                               // declval, move, pair
#include <vector>                                // vector

// The candidates for the set reading, the standard library's models first, and the inputs a clause checks them over.
namespace test::spec::set {

using models = std::tuple<std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET

                          ,
                          std::flat_set<std::size_t>

#endif
                          >;

// Every Block empty, at a single bit, either side of its first two block boundaries, at 17 and 24, and far past.
using fixed = std::tuple<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 0>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 1>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 7>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 8>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 9>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 15>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 16>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 17>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 24>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 257>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 0>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 1>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 15>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 16>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 17>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 24>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 31>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 32>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 33>, xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 48>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 0>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 1>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 17>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 24>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 31>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 32>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 33>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 63>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 64>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 65>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 1023>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 0>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 1>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 17>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 24>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 63>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 64>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 65>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 1025>
#ifdef TEST_HAS_UINT128

                         ,
                         xstd::basic_bit_fixed_set<std::size_t, xstd::uint128, 0>, xstd::basic_bit_fixed_set<std::size_t, xstd::uint128, 1>, xstd::basic_bit_fixed_set<std::size_t, xstd::uint128, 17>, xstd::basic_bit_fixed_set<std::size_t, xstd::uint128, 24>, xstd::basic_bit_fixed_set<std::size_t, xstd::uint128, 127>, xstd::basic_bit_fixed_set<std::size_t, xstd::uint128, 128>, xstd::basic_bit_fixed_set<std::size_t, xstd::uint128, 129>, xstd::basic_bit_fixed_set<std::size_t, xstd::uint128, 2049>

#endif
                         >;

using dynamic = std::tuple<xstd::basic_bit_set<std::size_t, std::uint8_t>, xstd::basic_bit_set<std::size_t, std::uint64_t>>;

using bounded = std::tuple<xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 0>, xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 9>, xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 17>, xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 24>, xstd::basic_bit_bounded_set<std::size_t, std::uint64_t, 24>, xstd::basic_bit_bounded_set<std::size_t, std::uint64_t, 65>, xstd::basic_bit_bounded_set<std::size_t, std::uint64_t, 4097>>;

// Inline blocks that a sweep to its limit spills from, that a sweep fits in, and that hold half of what a sample spans.
using small = std::tuple<xstd::basic_bit_small_set<std::size_t, std::uint8_t, 9>, xstd::basic_bit_small_set<std::size_t, std::uint64_t, 64>, xstd::basic_bit_small_set<std::size_t, std::uint64_t, 1024>>;

// Storage written outside the library, adapted by the same set adaptor the owners derive from.
using user_storage = std::tuple<xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_container<test::minimal_blocks<std::uint8_t>>>>;

// The columns once more under std::greater, std::set's model first: an empty width, across blocks, and growing.
using descending = std::tuple<std::set<std::size_t, std::greater<std::size_t>>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 0, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 17, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 64, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>, xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 65, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>, xstd::basic_bit_set<std::size_t, std::uint8_t, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>, xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 17, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>, xstd::basic_bit_small_set<std::size_t, std::uint8_t, 9, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>, xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_container<test::minimal_blocks<std::uint8_t>>, xstd::bits::detail::storage::owned, void, std::size_t, xstd::bit_key_traits<std::size_t>, std::greater<std::size_t>>>; // NOLINT(modernize-use-transparent-functors): std::set<std::size_t, std::greater<std::size_t>>'s comparator, as written

// Everything that owns the keys it holds, which is what [container.requirements] and a constructor ask for.
using owners = decltype(std::tuple_cat(std::declval<models>(), std::declval<fixed>(), std::declval<dynamic>(), std::declval<bounded>(), std::declval<small>(), std::declval<user_storage>(), std::declval<descending>()));

// Keys another object owns, with no std model: [set] less what owning implies; a clause they lack takes owners.
using views = std::tuple<xstd::bit_set_view<std::array<std::uint8_t, 3>, 17>, xstd::bit_set_view<std::vector<std::uint64_t>>>;

using all = decltype(std::tuple_cat(std::declval<owners>(), std::declval<views>()));

// The same views over const blocks, which read the keys and write none: a clause asks them only what they lack.
using const_views = std::tuple<xstd::bit_set_view<std::array<std::uint8_t, 3> const, 17>, xstd::bit_set_view<std::vector<std::uint64_t> const>>;

// The owners that take an allocator, under one that keeps a ledger and refuses on request, std::set first.
using ledgered = std::tuple<std::set<std::size_t, std::less<>, test::container::non_propagating<std::size_t>>, xstd::basic_bit_set<std::size_t, std::uint8_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, test::container::non_propagating<std::uint8_t>>, xstd::basic_bit_set<std::size_t, std::uint64_t, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, test::container::non_propagating<std::uint64_t>>, xstd::basic_bit_small_set<std::size_t, std::uint8_t, 9, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, test::container::non_propagating<std::uint8_t>>, xstd::basic_bit_small_set<std::size_t, std::uint64_t, 64, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, test::container::non_propagating<std::uint64_t>>>; // NOLINT(modernize-use-transparent-functors): the default comparator, named to reach the allocator

} // namespace test::spec::set

namespace test::spec {

// A set view over the whole of a fixed or a growing set; it has no windows, so the width is the owner's.
template<class Block, std::size_t K, std::size_t N>
struct view_traits<xstd::bit_set_view<std::array<Block, K>, N>>
{
        using owner_type = xstd::basic_bit_fixed_set<std::size_t, Block, N>;

        [[nodiscard]] static auto view(owner_type& owner, std::size_t)
        {
                return xstd::bit_set_view(owner);
        }
};

template<class Block, class Allocator>
struct view_traits<xstd::bit_set_view<std::vector<Block, Allocator>, std::dynamic_extent>>
{
        using owner_type = xstd::basic_bit_set<std::size_t, Block, xstd::bit_key_traits<std::size_t>, std::less<std::size_t>, Allocator>; // NOLINT(modernize-use-transparent-functors): the default comparator, named to reach the allocator

        [[nodiscard]] static auto view(owner_type& owner, std::size_t)
        {
                return xstd::bit_set_view(owner);
        }
};

} // namespace test::spec

namespace test::spec::set {

// The keys a set holds without growing: a width or a capacity in its type, a small set's inline blocks, or none at all.
template<class X>
inline constexpr auto held_width_v = [] -> std::size_t {
        if constexpr (test::set::static_width<X>) {
                // NOLINTNEXTLINE(readability-static-accessed-through-instance): a function on the standard's owners.
                return X().max_size();
        } else if constexpr (test::set::static_capacity<X>) {
                return xstd::bits::detail::owned_storage<X>::bits_type::static_capacity();
        } else {
                return 0UZ;
        }
}();

template<class Block, std::size_t N, class Compare, class Allocator>
inline constexpr auto held_width_v<xstd::basic_bit_small_set<std::size_t, Block, N, xstd::bit_key_traits<std::size_t>, Compare, Allocator>> = N;

#ifdef __clang__

// Clang shows a parameterless constexpr function can be constant by running it, here the whole enumeration.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-constexpr"

#endif

namespace inputs {

// A linear or quadratic enumeration runs up to 24 keys, a cubic or quartic one up to 17, an unbounded set's at a limit.
template<class X>
inline constexpr auto quadratic = held_width_v<owner_t<X>> <= 24UZ;

template<class X>
inline constexpr auto quartic = held_width_v<owner_t<X>> <= 17UZ;

// The keys of each enumeration, at the limits a type's sweep takes, shared by every type with the same limits.
namespace keys {

using test::set::on0::empty_set;
using test::set::on0::full_set;
using test::set::on1::all_cardinality_sets;
using test::set::on1::all_singleton_arrays;
using test::set::on1::all_singleton_sets;
using test::set::on1::all_valid;
using test::set::on2::all_doubleton_arrays;
using test::set::on2::all_doubleton_sets;
using test::set::on2::all_singleton_set_pairs;
using test::set::on3::all_singleton_set_triples;
using test::set::on4::all_doubleton_set_pairs;

[[nodiscard]] constexpr auto sets(std::size_t n, bool sweep)
        -> std::vector<one<key_vector>>
{
        auto result = std::vector<one<key_vector>>();
        empty_set<key_vector>([&](auto const& a) -> void { result.push_back({.from = edge("empty", n), .a = a}); });
        full_set<key_vector>([&](auto const& a) -> void { result.push_back({.from = edge("full", n), .a = a}); }, n);
        if (sweep) {
                all_cardinality_sets<key_vector>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every cardinality", n), .a = a}); }, n);
                all_singleton_sets<key_vector>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every singleton", n), .a = a}); }, n);
        }
        return result;
}

[[nodiscard]] constexpr auto doubletons(std::size_t n2)
        -> std::vector<one<key_vector>>
{
        auto result = std::vector<one<key_vector>>();
        all_doubleton_sets<key_vector>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every doubleton", n2), .a = a}); }, n2);
        return result;
}

[[nodiscard]] constexpr auto pairs(std::size_t n, std::size_t n2, bool sweep)
        -> std::vector<two<key_vector>>
{
        auto result = std::vector<two<key_vector>>();
        empty_set<key_vector>([&](auto const& e) -> void {
                full_set<key_vector>(
                        [&](auto const& f) -> void {
                                result.push_back({.from = edge("empty and empty", n), .a = e, .b = e});
                                result.push_back({.from = edge("empty and full", n), .a = e, .b = f});
                                result.push_back({.from = edge("full and empty", n), .a = f, .b = e});
                                result.push_back({.from = edge("full and full", n), .a = f, .b = f});
                        },
                        n
                );
        });
        if (sweep) {
                all_singleton_set_pairs<key_vector>([&](auto const& a, auto const& b) -> void { result.push_back({.from = exhaustive("every singleton pair", n2), .a = a, .b = b}); }, n2);
        }
        return result;
}

[[nodiscard]] constexpr auto doubleton_pairs(std::size_t n4)
        -> std::vector<two<key_vector>>
{
        auto result = std::vector<two<key_vector>>();
        all_doubleton_set_pairs<key_vector>([&](auto const& a, auto const& b) -> void { result.push_back({.from = exhaustive("every doubleton pair", n4), .a = a, .b = b}); }, n4);
        return result;
}

[[nodiscard]] constexpr auto triples(std::size_t n, std::size_t n3, bool sweep)
        -> std::vector<three<key_vector>>
{
        auto result = std::vector<three<key_vector>>();
        empty_set<key_vector>([&](auto const& e) -> void { result.push_back({.from = edge("empty thrice", n), .a = e, .b = e, .c = e}); });
        if (sweep) {
                all_singleton_set_triples<key_vector>([&](auto const& a, auto const& b, auto const& c) -> void { result.push_back({.from = exhaustive("every singleton triple", n3), .a = a, .b = b, .c = c}); }, n3);
        }
        return result;
}

[[nodiscard]] constexpr auto keyed_sets(std::size_t n, bool sweep)
        -> std::vector<keyed<key_vector>>
{
        auto result          = std::vector<keyed<key_vector>>();
        auto const with_keys = [&](char const* name, char const* every, key_vector const& a) -> void {
                if (sweep) {
                        all_valid<key_vector>([&](std::size_t k) -> void { result.push_back({.from = exhaustive(every, n), .a = a, .k = k}); }, n);
                } else if (n != 0) {
                        result.push_back({.from = edge(name, n), .a = a, .k = 0UZ});
                        result.push_back({.from = edge(name, n), .a = a, .k = n - 1UZ});
                }
        };
        empty_set<key_vector>([&](auto const& a) -> void { with_keys("empty", "empty with every key", a); });
        full_set<key_vector>([&](auto const& a) -> void { with_keys("full", "full with every key", a); }, n);
        return result;
}

[[nodiscard]] constexpr auto keyed_singletons(std::size_t n)
        -> std::vector<keyed<key_vector>>
{
        auto result = std::vector<keyed<key_vector>>();
        all_valid<key_vector>(
                [&](std::size_t k) -> void {
                        all_singleton_sets<key_vector>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every singleton with every key", n), .a = a, .k = k}); }, n);
                },
                n
        );
        return result;
}

[[nodiscard]] constexpr auto lists(std::size_t n, std::size_t n2, bool sweep)
        -> std::vector<key_list>
{
        auto result = std::vector<key_list>();
        result.push_back({.from = edge("no keys", n), .a = {}});
        full_set<key_vector>([&](auto const& a) -> void { result.push_back({.from = edge("every key", n), .a = a}); }, n);
        if (sweep) {
                all_singleton_arrays<key_vector>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every singleton", n), .a = key_vector(a.begin(), a.end())}); }, n);
                all_doubleton_arrays<key_vector>([&](auto const& a) -> void { result.push_back({.from = exhaustive("every doubleton", n2), .a = key_vector(a.begin(), a.end())}); }, n2);
        }
        return result;
}

[[nodiscard]] constexpr auto listed_sets(std::size_t n, std::size_t n2, bool sweep)
        -> std::vector<listed<key_vector>>
{
        auto result           = std::vector<listed<key_vector>>();
        auto const all_lists  = lists(n, n2, sweep);
        auto const with_lists = [&](auto const& a) -> void {
                for (auto const& [from, list] : all_lists) {
                        result.push_back({.from = from, .a = a, .keys = list});
                }
        };
        empty_set<key_vector>(with_lists);
        full_set<key_vector>(with_lists, n);
        return result;
}

[[nodiscard]] constexpr auto listed_singletons(std::size_t n)
        -> std::vector<listed<key_vector>>
{
        auto result = std::vector<listed<key_vector>>();
        all_singleton_sets<key_vector>(
                [&](auto const& a) -> void {
                        all_singleton_arrays<key_vector>([&](auto const& k) -> void { result.push_back({.from = exhaustive("every singleton with every singleton", n), .a = a, .keys = key_vector(k.begin(), k.end())}); }, n);
                },
                n
        );
        return result;
}

// A sampled set with the keys of another, in ascending order.
[[nodiscard]] inline auto sampled_listed(std::size_t n, std::size_t digits)
        -> std::vector<listed<key_vector>>
{
        auto result = std::vector<listed<key_vector>>();
        for (auto& [from, a, b] : random::pair_samples(n, digits)) {
                std::ranges::sort(b);
                result.push_back({.from = from, .a = std::move(a), .keys = std::move(b)});
        }
        return result;
}

} // namespace keys

template<class X>
inline constexpr auto n1 = test::set::limit_v<owner_t<X>, test::set::L1>;

template<class X>
inline constexpr auto n2 = test::set::limit_v<owner_t<X>, test::set::L2>;

template<class X>
inline constexpr auto n3 = test::set::limit_v<owner_t<X>, test::set::L3>;

template<class X>
inline constexpr auto n4 = test::set::limit_v<owner_t<X>, test::set::L4>;

template<class X>
struct set_of
{
        [[nodiscard]] constexpr auto operator()(key_vector const& keys) const
                -> X
        {
                return X(keys.begin(), keys.end());
        }
};

// A view's keys in an owner of its own, which the view is taken over.
template<view_type X>
struct set_of<X>
{
        [[nodiscard]] auto operator()(key_vector const& keys) const
                -> viewed<X>
        {
                return viewed<X>(set_of<owner_t<X>>()(keys), 0UZ);
        }
};

template<class X, class Carrier>
using over = rebuilt<input_t<X>, Carrier, set_of<X>>;

// A list of keys handed over as it is.
struct keys_of
{
        [[nodiscard]] constexpr auto operator()(key_vector const& keys) const
                -> key_vector
        {
                return keys;
        }
};

// The empty and the full set, every cardinality, and every singleton.
template<class X>
[[nodiscard]] constexpr auto fixed_sets()
        -> over<X, one<key_vector>>
{
        auto result = over<X, one<key_vector>>();
        result.append(keys::sets(n1<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_sets()
        -> over<X, one<key_vector>>
{
        auto result = over<X, one<key_vector>>();
        result.share(memo<&random::key_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto sets()
        -> over<X, one<key_vector>>
{
        auto result = over<X, one<key_vector>>();
        result.share(memo<&keys::sets>(n1<X>, quadratic<X>));
        result.share(memo<&random::key_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// Every doubleton as well.
template<class X>
[[nodiscard]] constexpr auto fixed_sets_with_doubletons()
        -> over<X, one<key_vector>>
{
        auto result = over<X, one<key_vector>>();
        result.append(keys::sets(n1<X>, quadratic<X>));
        if constexpr (quadratic<X>) {
                result.append(keys::doubletons(n2<X>));
        }
        return result;
}

template<class X>
[[nodiscard]] auto random_sets_with_doubletons()
        -> over<X, one<key_vector>>
{
        auto result = over<X, one<key_vector>>();
        result.share(memo<&random::key_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto sets_with_doubletons()
        -> over<X, one<key_vector>>
{
        auto result = over<X, one<key_vector>>();
        result.share(memo<&keys::sets>(n1<X>, quadratic<X>));
        if constexpr (quadratic<X>) {
                result.share(memo<&keys::doubletons>(n2<X>));
        }
        result.share(memo<&random::key_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// The empty and the full set in each order and against themselves, and every pair of singletons.
template<class X>
[[nodiscard]] constexpr auto fixed_pairs()
        -> over<X, two<key_vector>>
{
        auto result = over<X, two<key_vector>>();
        result.append(keys::pairs(n1<X>, n2<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_pairs()
        -> over<X, two<key_vector>>
{
        auto result = over<X, two<key_vector>>();
        result.share(memo<&random::pair_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto pairs()
        -> over<X, two<key_vector>>
{
        auto result = over<X, two<key_vector>>();
        result.share(memo<&keys::pairs>(n1<X>, n2<X>, quadratic<X>));
        result.share(memo<&random::pair_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// Every pair of doubletons as well, so each way four keys interleave, which an ordering or an inclusion turns on.
template<class X>
[[nodiscard]] constexpr auto fixed_pairs_with_doubletons()
        -> over<X, two<key_vector>>
{
        auto result = over<X, two<key_vector>>();
        result.append(keys::pairs(n1<X>, n2<X>, quadratic<X>));
        if constexpr (quartic<X>) {
                result.append(keys::doubleton_pairs(n4<X>));
        }
        return result;
}

template<class X>
[[nodiscard]] auto random_pairs_with_doubletons()
        -> over<X, two<key_vector>>
{
        auto result = over<X, two<key_vector>>();
        result.share(memo<&random::pair_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto pairs_with_doubletons()
        -> over<X, two<key_vector>>
{
        auto result = over<X, two<key_vector>>();
        result.share(memo<&keys::pairs>(n1<X>, n2<X>, quadratic<X>));
        if constexpr (quartic<X>) {
                result.share(memo<&keys::doubleton_pairs>(n4<X>));
        }
        result.share(memo<&random::pair_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// The empty set thrice, and every triple of singletons.
template<class X>
[[nodiscard]] constexpr auto fixed_triples()
        -> over<X, three<key_vector>>
{
        auto result = over<X, three<key_vector>>();
        result.append(keys::triples(n1<X>, n3<X>, quartic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_triples()
        -> over<X, three<key_vector>>
{
        auto result = over<X, three<key_vector>>();
        result.share(memo<&random::triple_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto triples()
        -> over<X, three<key_vector>>
{
        auto result = over<X, three<key_vector>>();
        result.share(memo<&keys::triples>(n1<X>, n3<X>, quartic<X>));
        result.share(memo<&random::triple_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// The empty and the full set with every key, or with the first and last where a sweep cannot afford them all.
template<class X>
[[nodiscard]] constexpr auto fixed_keyed_sets()
        -> over<X, keyed<key_vector>>
{
        auto result = over<X, keyed<key_vector>>();
        result.append(keys::keyed_sets(n1<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_keyed_sets()
        -> over<X, keyed<key_vector>>
{
        auto result = over<X, keyed<key_vector>>();
        result.share(memo<&random::keyed_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto keyed_sets()
        -> over<X, keyed<key_vector>>
{
        auto result = over<X, keyed<key_vector>>();
        result.share(memo<&keys::keyed_sets>(n1<X>, quadratic<X>));
        result.share(memo<&random::keyed_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// Every singleton with every key as well.
template<class X>
[[nodiscard]] constexpr auto fixed_keyed_sets_with_singletons()
        -> over<X, keyed<key_vector>>
{
        auto result = over<X, keyed<key_vector>>();
        result.append(keys::keyed_sets(n1<X>, quadratic<X>));
        if constexpr (quadratic<X>) {
                result.append(keys::keyed_singletons(n1<X>));
        }
        return result;
}

template<class X>
[[nodiscard]] auto random_keyed_sets_with_singletons()
        -> over<X, keyed<key_vector>>
{
        auto result = over<X, keyed<key_vector>>();
        result.share(memo<&random::keyed_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto keyed_sets_with_singletons()
        -> over<X, keyed<key_vector>>
{
        auto result = over<X, keyed<key_vector>>();
        result.share(memo<&keys::keyed_sets>(n1<X>, quadratic<X>));
        if constexpr (quadratic<X>) {
                result.share(memo<&keys::keyed_singletons>(n1<X>));
        }
        result.share(memo<&random::keyed_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// No keys, every key in ascending order, every list of one key and of two, and sampled keys in the order drawn.
template<class X>
[[nodiscard]] constexpr auto fixed_key_lists()
        -> rebuilt<key_vector, key_list, keys_of>
{
        auto result = rebuilt<key_vector, key_list, keys_of>();
        result.append(keys::lists(n1<X>, n2<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_key_lists()
        -> rebuilt<key_vector, key_list, keys_of>
{
        auto result = rebuilt<key_vector, key_list, keys_of>();
        result.share(memo<&random::key_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto key_lists()
        -> rebuilt<key_vector, key_list, keys_of>
{
        auto result = rebuilt<key_vector, key_list, keys_of>();
        result.share(memo<&keys::lists>(n1<X>, n2<X>, quadratic<X>));
        result.share(memo<&random::key_samples>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// The empty and the full set, each with every list of keys, and a sampled set with the ascending keys of another.
template<class X>
[[nodiscard]] constexpr auto fixed_listed_sets()
        -> over<X, listed<key_vector>>
{
        auto result = over<X, listed<key_vector>>();
        result.append(keys::listed_sets(n1<X>, n2<X>, quadratic<X>));
        return result;
}

template<class X>
[[nodiscard]] auto random_listed_sets()
        -> over<X, listed<key_vector>>
{
        auto result = over<X, listed<key_vector>>();
        result.share(memo<&keys::sampled_listed>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto listed_sets()
        -> over<X, listed<key_vector>>
{
        auto result = over<X, listed<key_vector>>();
        result.share(memo<&keys::listed_sets>(n1<X>, n2<X>, quadratic<X>));
        result.share(memo<&keys::sampled_listed>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

// Every singleton with every list of one key as well.
template<class X>
[[nodiscard]] constexpr auto fixed_listed_sets_with_singletons()
        -> over<X, listed<key_vector>>
{
        auto result = over<X, listed<key_vector>>();
        result.append(keys::listed_sets(n1<X>, n2<X>, quadratic<X>));
        if constexpr (quadratic<X>) {
                result.append(keys::listed_singletons(n1<X>));
        }
        return result;
}

template<class X>
[[nodiscard]] auto random_listed_sets_with_singletons()
        -> over<X, listed<key_vector>>
{
        auto result = over<X, listed<key_vector>>();
        result.share(memo<&keys::sampled_listed>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

template<class X>
[[nodiscard]] auto listed_sets_with_singletons()
        -> over<X, listed<key_vector>>
{
        auto result = over<X, listed<key_vector>>();
        result.share(memo<&keys::listed_sets>(n1<X>, n2<X>, quadratic<X>));
        if constexpr (quadratic<X>) {
                result.share(memo<&keys::listed_singletons>(n1<X>));
        }
        result.share(memo<&keys::sampled_listed>(random::width<owner_t<X>>(), random::block_digits_v<owner_t<X>>));
        return result;
}

} // namespace inputs

#ifdef __clang__

#pragma clang diagnostic pop

#endif

} // namespace test::spec::set

#endif // TEST_SPEC_SET_HPP
