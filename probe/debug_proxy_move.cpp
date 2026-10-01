//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <cstdio>
#include <exception>
#include <memory>
#include <new>
#include <utility>
#include <vector>

// Grants budget more allocations and then throws std::bad_alloc; -1 grants every one.
inline int budget = -1;
inline int calls = 0;

template<class T>
struct refusing
{
        using value_type = T;
        refusing() = default;
        template<class U>
        refusing(refusing<U> const&) noexcept
        {}
        auto allocate(std::size_t n) -> T*
        {
                ++calls;
                if (budget == 0) {
                        std::printf("  refusing allocation #%d of %zu x %zu bytes\n", calls, n, sizeof(T));
                        std::fflush(stdout);
                        throw std::bad_alloc();
                }
                if (budget > 0) {
                        --budget;
                }
                return std::allocator<T>().allocate(n);
        }
        auto deallocate(T* p, std::size_t n) noexcept -> void { std::allocator<T>().deallocate(p, n); }
        friend auto operator==(refusing, refusing) -> bool = default;
};

using V = std::vector<unsigned char, refusing<unsigned char>>;

auto main() -> int
{
        std::set_terminate([] {
                std::puts("  std::terminate called");
                std::fflush(stdout);
                std::_Exit(3);
        });
        auto v = V(10);
        std::printf("_ITERATOR_DEBUG_LEVEL=%d\n", _ITERATOR_DEBUG_LEVEL);
        calls = 0;
        auto w = std::move(v);
        std::printf("move construction made %d allocation(s)\n", calls);
        calls = 0;
        auto u = V(5);
        calls = 0;
        u = std::move(w);
        std::printf("move assignment (equal allocators) made %d allocation(s)\n", calls);
        std::puts("now refusing the next allocation inside a noexcept move construction:");
        std::fflush(stdout);
        budget = 0;
        try {
                auto x = std::move(u);
                std::printf("  survived, size %zu\n", x.size());
        } catch (std::bad_alloc const&) {
                std::puts("  caught bad_alloc");
        }
        budget = -1;
        std::puts("done");
}
