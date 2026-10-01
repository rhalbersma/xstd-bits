//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <cstdio>
#include <iterator>
#include <memory>
#include <new>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

// An allocator that grants `budget` more allocations and then throws std::bad_alloc.
inline int budget = 1'000'000;

template<class T>
struct refusing
{
        using value_type = T;
        refusing() = default;
        template<class U> refusing(refusing<U> const&) noexcept {}
        T* allocate(std::size_t n)
        {
                if (budget-- <= 0) throw std::bad_alloc();
                return std::allocator<T>{}.allocate(n);
        }
        void deallocate(T* p, std::size_t n) noexcept { std::allocator<T>{}.deallocate(p, n); }
        friend bool operator==(refusing, refusing) = default;
};

// 200 values read through std::istream_iterator: a single-pass input range.
template<class T>
std::istringstream input()
{
        std::string text;
        for (int i = 0; i < 200; ++i) text += "0 ";
        return std::istringstream(text);
}

template<class T, class Op>
void run(char const* name, Op op)
{
        for (int k = 0; k < 3; ++k) {
                std::vector<T, refusing<T>> v(10);
                auto const before = v.size();
                auto in = input<T>();
                budget = k;
                try {
                        op(v, std::istream_iterator<T>(in), std::istream_iterator<T>());
                        budget = 1'000'000;
                        std::printf("%-34s k=%d  completed, size %zu\n", name, k, v.size());
                } catch (std::bad_alloc const&) {
                        budget = 1'000'000;
                        std::printf("%-34s k=%d  bad_alloc, size %zu -> %zu%s\n", name, k, before, v.size(),
                                    v.size() == before ? "" : "   <-- partial effect");
                }
        }
}

int main()
{
        auto at_middle = [](auto& v, auto f, auto l) { v.insert(v.begin() + 5, f, l); };
        auto at_end    = [](auto& v, auto f, auto l) { v.insert(v.end(), f, l); };
        auto append    = [](auto& v, auto f, auto l) { v.append_range(std::ranges::subrange(f, l)); };
        run<int>("vector<int>  insert(begin()+5, ...)", at_middle);
        run<int>("vector<int>  insert(end(), ...)", at_end);
        run<int>("vector<int>  append_range", append);
        run<bool>("vector<bool> insert(begin()+5, ...)", at_middle);
        run<bool>("vector<bool> insert(end(), ...)", at_end);
        run<bool>("vector<bool> append_range", append);
}
