//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_SPAN_HPP
#define TEST_SPEC_SPAN_HPP

#include <test/spec/input.hpp>       // keyed, keyed_pair, memo, one, rebuilt, two
#include <test/spec/random.hpp>      // block_digits_v
#include <test/spec/sequence.hpp>    // bools, values
#include <test/spec/view.hpp>        // input_t, owner_t, view_traits, viewed
#include <xstd/bits/bit_array.hpp>   // basic_bit_array
#include <xstd/bits/bit_span.hpp>    // bit_span
#include <xstd/bits/bit_subspan.hpp> // bit_subspan
#include <xstd/bits/bit_vector.hpp>  // basic_bit_vector
#include <algorithm>                 // all_of, min
#include <array>                     // array
#include <cstddef>                   // size_t
#include <cstdint>                   // uint8_t, uint64_t
#include <iterator>                  // begin
#include <limits>                    // numeric_limits
#include <ranges>                    // iota
#include <span>                      // dynamic_extent, span
#include <tuple>                     // tuple, tuple_cat
#include <type_traits>               // is_assignable_v
#include <utility>                   // as_const, declval
#include <valarray>                  // valarray
#include <vector>                    // vector

// The candidates for [views.span], std::span<bool> first, and the inputs a clause checks them over.
namespace test::spec::span {

using models = std::tuple<std::span<bool>, std::span<bool, 0>, std::span<bool, 9>, std::span<bool, 65>, std::span<bool const>, std::span<bool const, 17>>;

// The whole of an owner's bits: at an empty width, within a block, across two, const, and at a run-time width.
using whole = std::tuple<xstd::bit_span<std::array<std::uint8_t, 0>, 0>, xstd::bit_span<std::array<std::uint8_t, 2>, 9>, xstd::bit_span<std::array<std::uint64_t, 2>, 65>, xstd::bit_span<std::array<std::uint8_t, 3> const, 17>, xstd::bit_span<std::vector<std::uint8_t>>, xstd::bit_span<std::vector<std::uint64_t>>>;

// A window three positions short of a block boundary, at a run-time and a static extent, over either storage.
using windows = std::tuple<xstd::bit_subspan<std::array<std::uint8_t, 3>, std::dynamic_extent, 24>, xstd::bit_subspan<std::array<std::uint8_t, 3>, 9, 24>, xstd::bit_subspan<std::array<std::uint8_t, 3> const, std::dynamic_extent, 24>, xstd::bit_subspan<std::vector<std::uint64_t>, std::dynamic_extent, std::dynamic_extent>, xstd::bit_subspan<std::vector<std::uint64_t>, 65, std::dynamic_extent>>;

using all = decltype(std::tuple_cat(std::declval<models>(), std::declval<whole>(), std::declval<windows>()));

// Whether a candidate is the standard library's, which may not yet declare every member the draft does.
template<class T>
inline constexpr bool is_model = false;

template<class E, std::size_t X>
inline constexpr bool is_model<std::span<E, X>> = true;

using test::spec::sequence::inputs::bools;

} // namespace test::spec::span

namespace test::spec {

// std::span<bool> views the bools of a valarray, whose begin is a contiguous iterator.
template<class E, std::size_t X>
struct view_traits<std::span<E, X>>
{
        using owner_type = std::valarray<bool>;

        [[nodiscard]] static auto owner(span::bools const& v)
                -> owner_type
        {
                auto result = owner_type(v.size());
                for (auto const i : std::views::iota(0UZ, v.size())) {
                        result[i] = v[i];
                }
                return result;
        }

        [[nodiscard]] static auto view(owner_type& owner, std::size_t width)
                -> std::span<E, X>
        {
                return std::span<E, X>(std::begin(owner), width);
        }
};

namespace span::detail {

// A packed owner's bits: the input's at the offset, and every position around them set, so a stray read shows.
template<class Owner>
[[nodiscard]] constexpr auto owner_at(bools const& v, std::size_t offset, std::size_t margin)
        -> Owner
{
        auto result = Owner();
        if constexpr (requires { result.resize(0UZ); }) {
                result.resize(offset + v.size() + margin, true);
        } else {
                result.fill(true);
        }
        for (auto const i : std::views::iota(0UZ, v.size())) {
                result[offset + i] = v[i];
        }
        return result;
}

// The whole of an owner, mutable or const, and a window three positions short of the owner's first block boundary.
template<class Owner, bool Const>
struct whole_traits
{
        using owner_type = Owner;

        [[nodiscard]] static constexpr auto owner(bools const& v)
                -> owner_type
        {
                return owner_at<Owner>(v, 0UZ, 0UZ);
        }

        [[nodiscard]] static constexpr auto view(owner_type& owner, std::size_t)
        {
                if constexpr (Const) {
                        return xstd::bit_span(std::as_const(owner));
                } else {
                        return xstd::bit_span(owner);
                }
        }
};

template<class Owner, bool Const, std::size_t E>
struct window_traits
{
        using owner_type = Owner;

        static constexpr auto offset = static_cast<std::size_t>(std::numeric_limits<typename Owner::adapted_type::block_type>::digits) - 3UZ;

        [[nodiscard]] static constexpr auto owner(bools const& v)
                -> owner_type
        {
                return owner_at<Owner>(v, offset, 3UZ);
        }

        [[nodiscard]] static constexpr auto view(owner_type& owner, std::size_t width)
        {
                auto const whole = whole_traits<Owner, Const>::view(owner, 0UZ);
                if constexpr (E == std::dynamic_extent) {
                        return whole.subspan(offset, width);
                } else {
                        return whole.template subspan<offset, E>();
                }
        }
};

} // namespace span::detail

template<class Block, std::size_t K, std::size_t N>
struct view_traits<xstd::bit_span<std::array<Block, K>, N>> : span::detail::whole_traits<xstd::basic_bit_array<Block, N>, false>
{};

template<class Block, std::size_t K, std::size_t N>
struct view_traits<xstd::bit_span<std::array<Block, K> const, N>> : span::detail::whole_traits<xstd::basic_bit_array<Block, N>, true>
{};

template<class Block, class Allocator>
struct view_traits<xstd::bit_span<std::vector<Block, Allocator>, std::dynamic_extent>> : span::detail::whole_traits<xstd::basic_bit_vector<Block, Allocator>, false>
{};

template<class Block, std::size_t K, std::size_t E, std::size_t N>
struct view_traits<xstd::bit_subspan<std::array<Block, K>, E, N>> : span::detail::window_traits<xstd::basic_bit_array<Block, N>, false, E>
{};

template<class Block, std::size_t K, std::size_t E, std::size_t N>
struct view_traits<xstd::bit_subspan<std::array<Block, K> const, E, N>> : span::detail::window_traits<xstd::basic_bit_array<Block, N>, true, E>
{};

template<class Block, class Allocator, std::size_t E>
struct view_traits<xstd::bit_subspan<std::vector<Block, Allocator>, E, std::dynamic_extent>> : span::detail::window_traits<xstd::basic_bit_vector<Block, Allocator>, false, E>
{};

} // namespace test::spec

namespace test::spec::span {

// The bools between two iterators, read through whatever reference they hand out.
template<class I, class S>
[[nodiscard]] auto read(I first, S const& last)
        -> bools
{
        auto result = bools();
        for (; first != last; ++first) {
                result.push_back(static_cast<bool>(*first));
        }
        return result;
}

template<class R>
[[nodiscard]] auto read(R const& r)
        -> bools
{
        return read(r.begin(), r.end());
}

// Whether x and y are the same element: a write through x shows through y and is undone, or the two read alike.
template<class X, class Y>
[[nodiscard]] auto aliases(X&& x, Y&& y)
        -> bool
{
        auto const old = static_cast<bool>(x);
        if (static_cast<bool>(y) != old) {
                return false;
        }
        if constexpr (std::is_assignable_v<X&, bool>) {
                x               = not old;
                auto const seen = static_cast<bool>(y) == not old;
                x               = old;
                return seen;
        } else {
                return true;
        }
}

// Whether position i of x is position j of y.
template<class X, class Y>
[[nodiscard]] auto same_position(X const& x, std::size_t i, Y const& y, std::size_t j)
        -> bool
{
        return aliases(x[i], y[j]);
}

// Where a candidate's view starts in its owner: nought but for a window.
template<class T>
inline constexpr auto offset_v = [] -> std::size_t {
        if constexpr (requires { view_traits<T>::offset; }) {
                return view_traits<T>::offset;
        } else {
                return 0UZ;
        }
}();

// The bools an input's owner holds where its view is, read through the owner rather than through the view.
template<class T>
[[nodiscard]] auto owned_bools(viewed<T> const& a)
        -> bools
{
        auto result = bools();
        for (auto const i : std::views::iota(0UZ, a.width())) {
                result.push_back(static_cast<bool>(a.owner()[offset_v<T> + i]));
        }
        return result;
}

// Whether every position of an input's owner around its view is still set, as the owner was built.
template<class T>
[[nodiscard]] auto margins_set(viewed<T> const& a)
        -> bool
{
        auto const& owner = a.owner();
        return std::ranges::all_of(std::views::iota(0UZ, owner.size()), [&](std::size_t i) -> bool { return (i >= offset_v<T> and i < offset_v<T> + a.width()) or static_cast<bool>(owner[i]); });
}

// The positions a candidate views: an extent in its type, what its owner holds past a window's offset, or no bound.
template<class T>
inline constexpr auto room_v = [] -> std::size_t {
        if constexpr (T::extent != std::dynamic_extent) {
                return T::extent;
        } else if constexpr (requires { view_traits<T>::offset; }) {
                // A run-time width holds nothing until grown, and the margin takes three positions past the window.
                // NOLINTNEXTLINE(readability-static-accessed-through-instance): a function on the standard's owners.
                constexpr auto n = owner_t<T>().size();
                return n == 0UZ ? 0UZ : n - view_traits<T>::offset - 3UZ;
        } else {
                return 0UZ;
        }
}();

#ifdef __clang__

// Clang shows a parameterless constexpr function can be constant by running it, here the whole enumeration.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-constexpr"

#endif

namespace inputs {

namespace values = test::spec::sequence::inputs::values;

// A width a view takes at run time, which a sweep varies, where a static one is the type's.
template<class X>
inline constexpr auto grows = X::extent == std::dynamic_extent;

// A linear enumeration runs up to 64 positions, a quadratic one up to 32 and a cubic one up to 17.
template<class X>
inline constexpr auto linear = room_v<X> <= 64UZ;

template<class X>
inline constexpr auto quadratic = room_v<X> <= 32UZ;

template<class X>
inline constexpr auto cubic = room_v<X> <= 17UZ;

// The limits a sweep takes: 128 positions for the edges, then 64, 32 and 16 by its order, capped by the type.
template<class X, std::size_t Limit>
inline constexpr auto limit_v = not grows<X> ? room_v<X> : (room_v<X> == 0UZ ? Limit : std::ranges::min(room_v<X>, Limit));

template<class X>
inline constexpr auto n0 = limit_v<X, 128UZ>;

template<class X>
inline constexpr auto n1 = limit_v<X, 64UZ>;

template<class X>
inline constexpr auto n2 = limit_v<X, 32UZ>;

template<class X>
inline constexpr auto n3 = limit_v<X, 16UZ>;

// A sample is as wide as the type's room, and one past 2048 bits where a view has no bound.
template<class X>
inline constexpr auto sampled_width = limit_v<X, 2049UZ>;

template<class X>
inline constexpr auto digits = test::spec::random::block_digits_v<owner_t<X>>;

// Each input's bools in an owner of its own, which the candidate views.
template<class X>
struct span_of
{
        [[nodiscard]] auto operator()(bools const& v) const
                -> viewed<X>
        {
                return viewed<X>(view_traits<X>::owner(v), v.size());
        }
};

template<class X, class Carrier>
using over = rebuilt<input_t<X>, Carrier, span_of<X>>;

// The empty and the full view, every width, every prefix and every singleton, and random ones.
template<class X>
[[nodiscard]] auto views()
        -> over<X, one<bools>>
{
        auto result = over<X, one<bools>>();
        result.share(memo<&values::sequences>(grows<X>, n0<X>, n1<X>, linear<X>));
        result.share(memo<&values::sampled_sequences>(sampled_width<X>, digits<X>));
        return result;
}

// The empty and the full view in each order and against themselves, every pair of singletons and of widths.
template<class X>
[[nodiscard]] auto pairs()
        -> over<X, two<bools>>
{
        auto result = over<X, two<bools>>();
        result.share(memo<&values::pairs>(grows<X>, n0<X>, n2<X>, quadratic<X>));
        result.share(memo<&values::sampled_pairs>(sampled_width<X>, digits<X>));
        return result;
}

// The full view at its ends, every width at every position it has, and random ones at their ends and one drawn.
template<class X>
[[nodiscard]] auto indexed()
        -> over<X, keyed<bools>>
{
        auto result = over<X, keyed<bools>>();
        result.share(memo<&values::indexed>(grows<X>, n0<X>, n1<X>, linear<X>));
        result.share(memo<&values::sampled_indexed>(sampled_width<X>, digits<X>));
        return result;
}

// Every width, offset and count; random ones cut short where the width varies, and from a drawn position where not.
template<class X>
[[nodiscard]] auto spans()
        -> over<X, keyed_pair<bools>>
{
        auto result = over<X, keyed_pair<bools>>();
        result.share(memo<&values::spans>(grows<X>, n3<X>, cubic<X>));
        if constexpr (grows<X>) {
                result.share(memo<&values::sampled_spans>(sampled_width<X>, digits<X>));
        } else {
                result.share(memo<&values::sampled_index_pairs>(sampled_width<X>, digits<X>));
        }
        return result;
}

} // namespace inputs

#ifdef __clang__

#pragma clang diagnostic pop

#endif

} // namespace test::spec::span

#endif // TEST_SPEC_SPAN_HPP
