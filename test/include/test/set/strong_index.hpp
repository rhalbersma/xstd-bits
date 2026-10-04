//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_STRONG_INDEX_HPP
#define TEST_SET_STRONG_INDEX_HPP

#include <xstd/bits/bit_key_traits.hpp> // bit_key_traits
#include <boost/test/unit_test.hpp>     // BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                    // ranges::equal
#include <compare>                      // strong_ordering
#include <cstddef>                      // size_t
#include <format>                       // format, formatter
#include <ranges>                       // from_range, iota, reverse, to, transform
#include <set>                          // erase_if, set
#include <string>                       // string
#include <vector>                       // vector

// A key that is not a std::size_t, and the two ways a set owner is told where it goes.
namespace test::set {

// A strong index: ordered, never converting to or from std::size_t on its own.
struct strong_index
{
        std::size_t value;

        [[nodiscard]] friend auto operator==(strong_index, strong_index) noexcept -> bool = default;
        [[nodiscard]] friend auto operator<=>(strong_index, strong_index) noexcept -> std::strong_ordering = default;
};

// Positions start at the key First, in a universe of N keys: a mapping that is not the identity, and closes.
template<std::size_t First, std::size_t N>
struct offset_traits
{
        static constexpr auto size = N;

        [[nodiscard]] static constexpr auto to_index(strong_index key) noexcept
                -> std::size_t
        {
                return key.value - First;
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> strong_index
        {
                return {.value = index + First};
        }
};

} // namespace test::set

// The default traits, specialized for the key, as a strong index type's author would write them.
template<>
struct xstd::bit_key_traits<test::set::strong_index>
{
        [[nodiscard]] static constexpr auto to_index(test::set::strong_index key) noexcept
                -> std::size_t
        {
                return key.value;
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> test::set::strong_index
        {
                return {.value = index};
        }
};

// Printed with a marker, so a set that formats its positions rather than its keys shows it.
template<class CharT>
struct std::formatter<test::set::strong_index, CharT> : std::formatter<std::size_t, CharT>
{
        template<class Context>
        [[nodiscard]] auto format(test::set::strong_index key, Context& ctx) const
        {
                return std::format_to(ctx.out(), "#{}", key.value);
        }
};

namespace test::set {

// A set keyed by strong_index against std::set<strong_index>, over keys the set can hold and a probe one past them.
template<class X>
auto agrees_with_std_set_of_strong_indices(std::vector<std::size_t> const& values, std::size_t first, std::size_t past)
        -> void
{
        using key = strong_index;
        auto const keys = values | std::views::transform([](std::size_t v) -> key { return {.value = v}; }) | std::ranges::to<std::vector>();
        auto const model = std::set<key>(keys.begin(), keys.end());
        auto const same = [](X const& x, std::set<key> const& m) -> bool { return std::ranges::equal(x, m); };

        // Construction, each way std::set has, and iteration in both directions.
        auto const a = X(keys.begin(), keys.end());
        BOOST_CHECK(same(a, model));
        BOOST_CHECK(same(X(std::from_range, keys), model));
        BOOST_CHECK(std::ranges::equal(a | std::views::reverse, model | std::views::reverse));
        BOOST_CHECK_EQUAL(a.size(), model.size());
        if (not model.empty()) {
                BOOST_CHECK(a.front() == *model.begin());
                BOOST_CHECK(a.back() == *model.rbegin());
        }

        // The walks hand out keys, as the iterators do.
        auto walked = std::vector<key>();
        a.for_each([&](key k) -> void { walked.push_back(k); });
        BOOST_CHECK(std::ranges::equal(walked, model));
        walked.clear();
        a.for_each_reverse([&](key k) -> void { walked.push_back(k); });
        BOOST_CHECK(std::ranges::equal(walked, model | std::views::reverse));

        // Lookup over every key of the universe and one past it, total as std::set's is.
        for (auto const v : std::views::iota(first, past + 1UZ)) {
                auto const k = key{.value = v};
                BOOST_CHECK_EQUAL(a.contains(k), model.contains(k));
                BOOST_CHECK_EQUAL(a.count(k), model.count(k));
                BOOST_CHECK_EQUAL(a.find(k) == a.end(), model.find(k) == model.end());
                auto const lb = a.lower_bound(k);
                auto const mlb = model.lower_bound(k);
                BOOST_CHECK_EQUAL(lb == a.end(), mlb == model.end());
                if (mlb != model.end()) {
                        BOOST_CHECK(*lb == *mlb);
                }
                auto const ub = a.upper_bound(k);
                auto const mub = model.upper_bound(k);
                BOOST_CHECK_EQUAL(ub == a.end(), mub == model.end());
                if (mub != model.end()) {
                        BOOST_CHECK(*ub == *mub);
                }
                auto const [lo, hi] = a.equal_range(k);
                BOOST_CHECK(lo == lb);
                BOOST_CHECK(hi == ub);
        }

        // The modifiers, one key at a time, each against the model.
        for (auto const v : std::views::iota(first, past)) {
                auto const k = key{.value = v};
                auto x = a;
                auto m = model;
                auto const [it, inserted] = x.insert(k);
                auto const [mit, minserted] = m.insert(k);
                BOOST_CHECK_EQUAL(inserted, minserted);
                BOOST_CHECK(*it == *mit);
                BOOST_CHECK(same(x, m));

                BOOST_CHECK(*x.insert(x.begin(), k) == k);
                BOOST_CHECK(x.emplace(k).first == x.find(k));
                BOOST_CHECK(*x.emplace_hint(x.end(), k) == k);
                BOOST_CHECK(same(x, m));

                BOOST_CHECK_EQUAL(x.erase(k), m.erase(k));
                BOOST_CHECK_EQUAL(x.erase(k), m.erase(k));
                BOOST_CHECK(same(x, m));

                x.complement(k);
                BOOST_CHECK(x.contains(k));
                x.complement(k);
                BOOST_CHECK(not x.contains(k));
        }

        // Erasure by iterator and by range, and by predicate.
        if (not model.empty()) {
                auto x = a;
                auto m = model;
                BOOST_CHECK_EQUAL(x.erase(x.begin()) == x.end(), m.erase(m.begin()) == m.end());
                BOOST_CHECK(same(x, m));
                BOOST_CHECK(x.erase(x.begin(), x.end()) == x.end());
                BOOST_CHECK(x.empty());
        }
        auto x = a;
        auto m = model;
        auto const odd = [](key k) -> bool { return k.value % 2UZ == 1UZ; };
        BOOST_CHECK_EQUAL(erase_if(x, odd), std::erase_if(m, odd));
        BOOST_CHECK(same(x, m));

        // The bulk inserts, and the list forms.
        auto y = X();
        y.insert(keys.begin(), keys.end());
        BOOST_CHECK(y == a);
        auto z = X();
        z.insert_range(keys);
        BOOST_CHECK(z == a);
        auto const k0 = key{.value = first};
        auto w = X({k0});
        w.insert({k0});
        w = {k0};
        BOOST_CHECK(same(w, std::set<key>{k0}));

        // The key, not its position, is what is printed.
        auto expected = std::string("{");
        for (auto const& k : model) {
                expected += std::format("{}#{}", k == *model.begin() ? "" : ", ", k.value);
        }
        BOOST_CHECK_EQUAL(std::format("{}", a), expected + "}");
}

} // namespace test::set

#endif // TEST_SET_STRONG_INDEX_HPP
