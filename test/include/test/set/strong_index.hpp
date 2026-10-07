//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_STRONG_INDEX_HPP
#define TEST_SET_STRONG_INDEX_HPP

#include <xstd/bits/bit_key_mapping.hpp> // bit_key_mapping
#include <boost/test/unit_test.hpp>      // BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                     // ranges::equal
#include <compare>                       // strong_ordering
#include <cstddef>                       // size_t
#include <format>                        // format, formatter
#include <ranges>                        // from_range, iota, reverse, to, transform
#include <set>                           // erase_if, set
#include <string>                        // string
#include <vector>                        // vector

// A key that is not a std::size_t, and the two ways a set owner is told where it goes.
namespace test::set {

// A strong index: ordered, never converting to or from std::size_t on its own.
struct strong_index
{
        std::size_t value;

        [[nodiscard]] friend auto operator==(strong_index, strong_index) -> bool                  = default;
        [[nodiscard]] friend auto operator<=>(strong_index, strong_index) -> std::strong_ordering = default;
};

// Positions start at the key First, in a universe of N keys: a mapping that is not the identity, and closes.
template<std::size_t First, std::size_t N>
struct offset_mapping
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

// The default mapping, specialized for the key, as a strong index type's author would write it.
template<>
struct xstd::bit_key_mapping<test::set::strong_index>
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

// Construction each way std::set has, iteration in both directions, and the walks, which hand out keys.
template<class X>
auto constructs_and_walks_as_std_set(std::vector<strong_index> const& keys, std::set<strong_index> const& model)
        -> void
{
        auto const a = X(keys.begin(), keys.end());
        BOOST_CHECK(std::ranges::equal(a, model));
        BOOST_CHECK(std::ranges::equal(X(std::from_range, keys), model));
        BOOST_CHECK(std::ranges::equal(a | std::views::reverse, model | std::views::reverse));
        BOOST_CHECK_EQUAL(a.size(), model.size());
        if (not model.empty()) {
                BOOST_CHECK(a.front() == *model.begin());
                BOOST_CHECK(a.back() == *model.rbegin());
        }

        auto walked = std::vector<strong_index>();
        a.for_each([&](strong_index k) -> void { walked.push_back(k); });
        BOOST_CHECK(std::ranges::equal(walked, model));
        walked.clear();
        a.for_each_reverse([&](strong_index k) -> void { walked.push_back(k); });
        BOOST_CHECK(std::ranges::equal(walked, model | std::views::reverse));
}

// Lookup of one key, total as std::set's is.
template<class X>
auto looks_up_as_std_set(X const& a, std::set<strong_index> const& model, strong_index k)
        -> void
{
        BOOST_CHECK_EQUAL(a.contains(k), model.contains(k));
        BOOST_CHECK_EQUAL(a.count(k), model.contains(k) ? 1UZ : 0UZ);
        auto const lb = a.lower_bound(k);
        BOOST_CHECK(a.find(k) == (model.contains(k) ? lb : a.end()));
        auto const mlb = model.lower_bound(k);
        BOOST_CHECK_EQUAL(lb == a.end(), mlb == model.end());
        BOOST_CHECK(mlb == model.end() or *lb == *mlb);
        auto const ub  = a.upper_bound(k);
        auto const mub = model.upper_bound(k);
        BOOST_CHECK_EQUAL(ub == a.end(), mub == model.end());
        BOOST_CHECK(mub == model.end() or *ub == *mub);
        auto const [lo, hi] = a.equal_range(k);
        BOOST_CHECK(lo == lb);
        BOOST_CHECK(hi == ub);
}

// The modifiers taking one key, each against the model.
template<class X>
auto modifies_as_std_set(X const& a, std::set<strong_index> const& model, strong_index k)
        -> void
{
        auto x                      = a;
        auto m                      = model;
        auto const [it, inserted]   = x.insert(k);
        auto const [mit, minserted] = m.insert(k);
        BOOST_CHECK_EQUAL(inserted, minserted);
        BOOST_CHECK(*it == *mit);
        BOOST_CHECK(std::ranges::equal(x, m));

        BOOST_CHECK(*x.insert(x.begin(), k) == k);
        BOOST_CHECK(x.emplace(k).first == x.find(k));
        BOOST_CHECK(*x.emplace_hint(x.end(), k) == k);
        BOOST_CHECK(std::ranges::equal(x, m));

        BOOST_CHECK_EQUAL(x.erase(k), m.erase(k));
        BOOST_CHECK_EQUAL(x.erase(k), m.erase(k));
        BOOST_CHECK(std::ranges::equal(x, m));

        x.complement(k);
        BOOST_CHECK(x.contains(k));
        x.complement(k);
        BOOST_CHECK(not x.contains(k));
}

// Erasure by iterator, by range and by predicate, the bulk inserts, the list forms, and the key as printed.
template<class X>
auto erases_inserts_and_prints_as_std_set(std::vector<strong_index> const& keys, std::set<strong_index> const& model, std::size_t first)
        -> void
{
        auto const a = X(keys.begin(), keys.end());
        if (not model.empty()) {
                auto x = a;
                auto m = model;
                BOOST_CHECK_EQUAL(x.erase(x.begin()) == x.end(), m.erase(m.begin()) == m.end());
                BOOST_CHECK(std::ranges::equal(x, m));
                BOOST_CHECK(x.erase(x.begin(), x.end()) == x.end());
                BOOST_CHECK(x.empty());
        }
        auto x         = a;
        auto m         = model;
        auto const odd = [](strong_index k) -> bool { return k.value % 2UZ == 1UZ; };
        BOOST_CHECK_EQUAL(erase_if(x, odd), std::erase_if(m, odd));
        BOOST_CHECK(std::ranges::equal(x, m));

        auto y = X();
        y.insert(keys.begin(), keys.end());
        BOOST_CHECK(y == a);
        auto z = X();
        z.insert_range(keys);
        BOOST_CHECK(z == a);
        auto const k0 = strong_index{.value = first};
        auto w        = X({k0});
        w.insert({k0});
        w = {k0};
        BOOST_CHECK(std::ranges::equal(w, std::set<strong_index>{k0}));

        auto expected = std::string("{");
        for (auto const& k : model) {
                expected += std::format("{}#{}", k == *model.begin() ? "" : ", ", k.value);
        }
        BOOST_CHECK_EQUAL(std::format("{}", a), expected + "}");
}

// A set keyed by strong_index against std::set<strong_index>, over keys the set can hold and a probe one past them.
template<class X>
auto agrees_with_std_set_of_strong_indices(std::vector<std::size_t> const& values, std::size_t first, std::size_t past)
        -> void
{
        auto const keys  = values | std::views::transform([](std::size_t v) -> strong_index { return {.value = v}; }) | std::ranges::to<std::vector>();
        auto const model = std::set<strong_index>(keys.begin(), keys.end());
        constructs_and_walks_as_std_set<X>(keys, model);

        auto const a = X(keys.begin(), keys.end());
        for (auto const v : std::views::iota(first, past + 1UZ)) {
                looks_up_as_std_set(a, model, strong_index{.value = v});
        }
        for (auto const v : std::views::iota(first, past)) {
                modifies_as_std_set(a, model, strong_index{.value = v});
        }
        erases_inserts_and_prints_as_std_set<X>(keys, model, first);
}

} // namespace test::set

#endif // TEST_SET_STRONG_INDEX_HPP
