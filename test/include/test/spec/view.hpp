//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_VIEW_HPP
#define TEST_SPEC_VIEW_HPP

#include <cstddef> // size_t
#include <utility> // move

// A view in a reading's list, and the owner an input keeps alive beside it for as long as the checks on it run.
namespace test::spec {

// Specialized by each reading for the views in its list: the owner_type, and view(owner, width) taking the view.
template<class V>
struct view_traits;

template<class V>
concept view_type = requires { typename view_traits<V>::owner_type; };

// The type an input holds: the candidate itself, or a view together with what it views.
template<class X>
struct input_type
{
        using type = X;
};

template<class V>
class viewed;

template<view_type V>
struct input_type<V>
{
        using type = viewed<V>;
};

template<class X>
using input_t = input_type<X>::type;

// The owner whose width and inputs a view takes: the candidate itself where it owns what it holds.
template<class X>
struct owner_of
{
        using type = X;
};

template<view_type V>
struct owner_of<V>
{
        using type = view_traits<V>::owner_type;
};

template<class X>
using owner_t = owner_of<X>::type;

#ifdef __clang__

// An owner is of any size, so the holder pads wherever its type puts it.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"

#endif

namespace detail {

// A base, so it is built before the view that refers into it; the width is what a window of the owner takes.
template<class Owner>
struct holds
{
        Owner m_owner;
        std::size_t m_width;
};

} // namespace detail

// A view over an owner of its own; a copy copies what is viewed, so a check's scratch copy leaves the input unchanged.
template<class V>
class viewed : private detail::holds<typename view_traits<V>::owner_type>, public V // NOLINT(misc-multiple-inheritance): the owner is built before the view that refers into it
{
        using traits = view_traits<V>;
        using holds  = detail::holds<typename traits::owner_type>;

public:
        using owner_type = traits::owner_type;

        [[nodiscard]] explicit viewed(owner_type owner, std::size_t width)
                : holds{.m_owner = std::move(owner), .m_width = width}
                , V(traits::view(holds::m_owner, width))
        {}

        [[nodiscard]] viewed(viewed const& other)
                : holds{.m_owner = other.m_owner, .m_width = other.m_width}
                , V(traits::view(holds::m_owner, other.m_width))
        {}

        // The view is taken anew, since what it refers to may have moved with the owner's new value.
        auto operator=(viewed const& other)
                -> viewed&
        {
                holds::m_owner         = other.m_owner;
                holds::m_width         = other.m_width;
                static_cast<V&>(*this) = traits::view(holds::m_owner, other.m_width);
                return *this;
        }

        ~viewed() = default;

        // The candidate itself, whose copy refers to the same owner as this one.
        [[nodiscard]] auto view() const
                -> V
        {
                return *this;
        }

        [[nodiscard]] auto owner() const noexcept
                -> owner_type const&
        {
                return holds::m_owner;
        }

        [[nodiscard]] auto width() const noexcept
                -> std::size_t
        {
                return holds::m_width;
        }
};

#ifdef __clang__

#pragma clang diagnostic pop

#endif

// A copy of the candidate an input holds, for what is asked of its type: itself, or the view a viewed input carries.
template<class X>
[[nodiscard]] auto subject(X const& x)
        -> X
{
        return x;
}

template<class V>
[[nodiscard]] auto subject(viewed<V> const& x)
        -> V
{
        return x.view();
}

} // namespace test::spec

#endif // TEST_SPEC_VIEW_HPP
