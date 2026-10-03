//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_CONTAINER_ALLOCATOR_HPP
#define TEST_CONTAINER_ALLOCATOR_HPP

#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <cstddef>                                  // size_t
#include <cstdint>                                  // int64_t, uint8_t
#include <iterator>                                 // ranges::distance
#include <memory>                                   // allocator
#include <new>                                      // bad_alloc
#include <ranges>                                   // iota
#include <set>                                      // set
#include <type_traits>                              // bool_constant, false_type, is_constructible_v
#include <utility>                                  // cmp_equal

// An allocator that keeps accounts and refuses on request, and the exception guarantees it lets a case observe.
namespace test::container {

// What an allocator has outstanding, how many more it grants, and what it refused or was handed back unasked.
struct ledger
{
        std::int64_t live = 0;
        std::int64_t budget = -1; // allocations still granted, and no limit while negative
        std::int64_t refusals = 0;
        std::int64_t strays = 0; // deallocations of what no allocator over this ledger allocated
        std::set<void const*> held;
};

// MSVC's debug containers allocate a bookkeeping proxy through the user's allocator, noexcept moves included.
template<class T>
inline constexpr bool is_debug_proxy_v = false;

#if defined(_ITERATOR_DEBUG_LEVEL) && _ITERATOR_DEBUG_LEVEL != 0

template<>
inline constexpr bool is_debug_proxy_v<std::_Container_proxy> = true;

#endif

// Stateful, so two instances can differ, and propagating on a copy, and unless told otherwise on a move and a swap.
template<class T, bool OnCopy, bool OnMove = OnCopy, bool OnSwap = OnCopy>
class tagged_allocator
{
        ledger* m_ledger = nullptr;
        std::int64_t m_tag = 0;

public:
        using value_type = T;
        using propagate_on_container_copy_assignment = std::bool_constant<OnCopy>;
        using propagate_on_container_move_assignment = std::bool_constant<OnMove>;
        using propagate_on_container_swap = std::bool_constant<OnSwap>;
        using is_always_equal = std::false_type;

        template<class U>
        struct rebind
        {
                using other = tagged_allocator<U, OnCopy, OnMove, OnSwap>;
        };

        [[nodiscard]] tagged_allocator() = default;

        [[nodiscard]] constexpr explicit tagged_allocator(int tag, ledger* book = nullptr) noexcept
                : m_ledger(book)
                , m_tag(tag)
        {}

        template<class U>
        [[nodiscard]] constexpr explicit(false) tagged_allocator(tagged_allocator<U, OnCopy, OnMove, OnSwap> const& other) noexcept
                : m_ledger(other.book())
                , m_tag(other.tag())
        {}

        [[nodiscard]] constexpr auto tag() const noexcept
                -> int
        {
                return static_cast<int>(m_tag);
        }

        [[nodiscard]] constexpr auto book() const noexcept
                -> ledger*
        {
                return m_ledger;
        }

        [[nodiscard]] auto allocate(std::size_t n)
                -> T*
        {
                if (m_ledger == nullptr) {
                        return std::allocator<T>().allocate(n);
                }
                // A debug proxy is accounted for but never refused: no guarantee the standard gives depends on it.
                if constexpr (not is_debug_proxy_v<T>) {
                        if (m_ledger->budget == 0) {
                                ++m_ledger->refusals;
                                throw std::bad_alloc();
                        }
                        if (m_ledger->budget > 0) {
                                --m_ledger->budget;
                        }
                }
                auto* const p = std::allocator<T>().allocate(n);
                m_ledger->held.insert(p);
                ++m_ledger->live;
                return p;
        }

        auto deallocate(T* p, std::size_t n) noexcept
                -> void
        {
                if (m_ledger != nullptr) {
                        if (m_ledger->held.erase(p) == 0UZ) {
                                ++m_ledger->strays;
                        } else {
                                --m_ledger->live;
                        }
                }
                std::allocator<T>().deallocate(p, n);
        }

        template<class U>
        [[nodiscard]] friend constexpr auto operator==(tagged_allocator const& lhs, tagged_allocator<U, OnCopy, OnMove, OnSwap> const& rhs) noexcept
                -> bool
        {
                return lhs.tag() == rhs.tag();
        }
};

template<class T>
using propagating = tagged_allocator<T, true>;

template<class T>
using non_propagating = tagged_allocator<T, false>;

// Propagating on a copy but not on a move, so that a swap hands the allocator over or nothing that cannot throw does.
template<class T>
using copy_and_swap_propagating = tagged_allocator<T, true, false, true>;

template<class T>
using copy_propagating = tagged_allocator<T, true, false, false>;

// The allocator the column was declared with, which the small columns wrap in one of Boost's own.
template<class X>
struct user_allocator
{
        using type = X::allocator_type;
};

template<class Block, std::size_t N, class Allocator>
struct user_allocator<xstd::basic_bit_small_set<Block, N, Allocator>>
{
        using type = Allocator;
};

template<class Block, std::size_t N, class Allocator>
struct user_allocator<xstd::basic_bit_small_vector<Block, N, Allocator>>
{
        using type = Allocator;
};

// An allocator that keeps its accounts in book, where the column takes an allocator and one that can.
template<class X>
concept keeps_a_ledger = requires { typename X::allocator_type; } and std::is_constructible_v<typename user_allocator<X>::type, int, ledger*>;

template<class X>
[[nodiscard]] auto ledger_allocator(ledger& book, int tag = 0)
        -> X::allocator_type
{
        return typename X::allocator_type(typename user_allocator<X>::type(tag, &book));
}

// How an operation ended under a budget: it completed, an allocation was refused, or it threw of its own accord.
enum class outcome : std::uint8_t
{
        completed,
        refused,
        raised,
};

// More refusals than any case's operation makes allocations: one that asks for more fails rather than runs on.
inline constexpr std::int64_t refusal_limit = 4096;

// op with granted allocations and the next one refused, and how it ended; the budget is lifted again either way.
template<class Op>
[[nodiscard]] auto run_granting(ledger& book, std::int64_t granted, Op op)
        -> outcome
{
        auto ended = outcome::completed;
        auto const refusals = book.refusals;
        book.budget = granted;
        try {
                op();
        } catch (...) {
                ended = book.refusals == refusals ? outcome::raised : outcome::refused;
        }
        book.budget = -1;
        return ended;
}

// What a valid object answers after a failed operation: iterators agreeing with its size, an equal copy, and a clear.
template<class X>
[[nodiscard]] auto is_valid(X& x)
        -> bool
{
        auto const copies = X(x) == x;
        auto const counts = [&] -> bool {
                if constexpr (requires { std::ranges::distance(x); }) {
                        return std::cmp_equal(std::ranges::distance(x), x.size());
                } else {
                        return true;
                }
        }();
        x.clear();
        return x.empty() and copies and counts;
}

// The strong guarantee: each refusal leaves a copy of a as a was, until op completes, and every allocation comes back.
template<class X, class Op>
[[nodiscard]] auto strong_guarantee(X const& a, Op op)
        -> bool
{
        if constexpr (keeps_a_ledger<X>) {
                auto book = ledger();
                auto holds = true;
                auto ended = outcome::refused;
                {
                        auto x = X(a, ledger_allocator<X>(book));
                        for (auto const granted : std::views::iota(std::int64_t{0}, refusal_limit)) {
                                ended = run_granting(book, granted, [&] -> void { op(x); });
                                if (ended == outcome::completed) {
                                        break;
                                }
                                holds = x == a and holds;
                                if (ended == outcome::raised) {
                                        break;
                                }
                        }
                }
                return ended != outcome::refused and book.live == 0 and book.strays == 0 and holds;
        } else {
                // No allocator to refuse: what op throws of its own accord, such as past a capacity, is all there is.
                auto x = a;
                try {
                        op(x);
                } catch (...) {
                        return x == a;
                }
                return true;
        }
}

// The basic guarantee: each refusal leaves a copy of a valid, until op completes, and every allocation comes back.
template<class X, class Op>
[[nodiscard]] auto basic_guarantee(X const& a, Op op)
        -> bool
{
        if constexpr (keeps_a_ledger<X>) {
                auto book = ledger();
                auto holds = true;
                for (auto const granted : std::views::iota(std::int64_t{0}, refusal_limit)) {
                        auto ended = outcome::completed;
                        {
                                auto x = X(a, ledger_allocator<X>(book));
                                ended = run_granting(book, granted, [&] -> void { op(x); });
                                if (ended != outcome::completed) {
                                        holds = is_valid(x) and holds;
                                }
                        }
                        holds = book.live == 0 and book.strays == 0 and holds;
                        if (ended != outcome::refused) {
                                return holds;
                        }
                }
                return false;
        } else {
                auto x = a;
                try {
                        op(x);
                } catch (...) {
                        return is_valid(x);
                }
                return true;
        }
}

} // namespace test::container

#endif // TEST_CONTAINER_ALLOCATOR_HPP
