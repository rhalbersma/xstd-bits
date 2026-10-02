//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef FUZZ_DECODER_HPP
#define FUZZ_DECODER_HPP

#include <cstddef>     // size_t
#include <cstdint>     // uint8_t
#include <cstdio>      // stderr
#include <cstdlib>     // abort
#include <print>       // println
#include <span>        // span
#include <tuple>       // tuple_element_t, tuple_size_v
#include <type_traits> // type_identity
#include <utility>     // forward, index_sequence, make_index_sequence

namespace fuzz {

// The fuzzer's bytes read front to back; every read is total, answering zero once the bytes run out.
class decoder
{
        std::span<std::uint8_t const> m_bytes;

public:
        [[nodiscard]] explicit decoder(std::span<std::uint8_t const> bytes) noexcept
                : m_bytes(bytes)
        {}

        [[nodiscard]] auto empty() const noexcept
                -> bool
        {
                return m_bytes.empty();
        }

        [[nodiscard]] auto byte() noexcept
                -> std::uint8_t
        {
                if (m_bytes.empty()) {
                        return 0;
                }
                auto const b = m_bytes.front();
                m_bytes = m_bytes.subspan(1);
                return b;
        }

        [[nodiscard]] auto boolean() noexcept
                -> bool
        {
                return (byte() & 1U) != 0U;
        }

        // A value in [0, bound) from two bytes, so every input is valid; the modulo bias is immaterial to a fuzzer.
        [[nodiscard]] auto below(std::size_t bound) noexcept
                -> std::size_t
        {
                auto const lo = static_cast<std::size_t>(byte());
                auto const hi = static_cast<std::size_t>(byte());
                return bound == 0UZ ? 0UZ : ((hi << 8U) | lo) % bound;
        }
};

// The operation being replayed, so a divergence names the step at which owner and model parted.
class checker
{
        std::size_t m_step{};
        char const* m_name{""};

public:
        auto step(char const* name) noexcept
                -> void
        {
                ++m_step;
                m_name = name;
        }

        auto expect(bool ok, char const* what) const noexcept
                -> void
        {
                if (not ok) {
                        std::println(stderr, "divergence at operation {} ({}): {}", m_step, m_name, what);
                        std::abort();
                }
        }
};

// Whether f throws an Exception; any other exception escapes to the fuzzer, which reports it as a crash.
template<class Exception, class F>
[[nodiscard]] auto throws(F&& f)
        -> bool
{
        try {
                std::forward<F>(f)();
        } catch (Exception const&) {
                return true;
        }
        return false;
}

// f called with the type of List at index, which the caller has reduced below the list's size.
template<class List, class F>
auto dispatch(std::size_t index, F&& f)
        -> void
{
        [&]<std::size_t... I>(std::index_sequence<I...>) -> void {
                static_cast<void>(((index == I ? (f(std::type_identity<std::tuple_element_t<I, List>>()), true) : false) or ...));
        }(std::make_index_sequence<std::tuple_size_v<List>>());
}

// The first byte picks a type of List, and the rest is that type's sequence of operations.
template<class List, class F>
auto run(std::uint8_t const* data, std::size_t size, F&& f)
        -> void
{
        auto in = decoder(std::span(data, size));
        dispatch<List>(in.byte() % std::tuple_size_v<List>, [&]<class T>(std::type_identity<T> type) -> void { f(type, in); });
}

} // namespace fuzz

#endif // FUZZ_DECODER_HPP
