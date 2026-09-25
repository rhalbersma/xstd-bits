//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_MINIMAL_WORDS_HPP
#define TEST_MINIMAL_WORDS_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // ptrdiff_t, size_t
#include <vector>                                  // vector

namespace test {

// Storage written outside the library with the members resizable_bit_storage names and no others.
template<xstd::unsigned_integer Block>
class minimal_words
{
        std::vector<Block> m_words;

public:
        using value_type = Block;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = Block&;
        using const_reference = Block const&;
        using iterator = std::vector<Block>::iterator;
        using const_iterator = std::vector<Block>::const_iterator;

        [[nodiscard]] friend auto operator==(minimal_words const&, minimal_words const&) -> bool = default;

        [[nodiscard]] constexpr auto begin() noexcept
                -> iterator
        {
                return m_words.begin();
        }

        [[nodiscard]] constexpr auto begin() const noexcept
                -> const_iterator
        {
                return m_words.begin();
        }

        [[nodiscard]] constexpr auto end() noexcept
                -> iterator
        {
                return m_words.end();
        }

        [[nodiscard]] constexpr auto end() const noexcept
                -> const_iterator
        {
                return m_words.end();
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                return m_words.size();
        }

        [[nodiscard]] constexpr auto max_size() const noexcept
                -> size_type
        {
                return m_words.max_size();
        }

        [[nodiscard]] constexpr auto operator[](size_type n) noexcept
                -> reference
        {
                return m_words[n];
        }

        [[nodiscard]] constexpr auto operator[](size_type n) const noexcept
                -> const_reference
        {
                return m_words[n];
        }

        constexpr auto resize(size_type n, Block word)
                -> void
        {
                m_words.resize(n, word);
        }

        constexpr auto push_back(Block word)
                -> void
        {
                m_words.push_back(word);
        }

        template<class InputIt>
        constexpr auto insert(const_iterator pos, InputIt first, InputIt last)
                -> iterator
        {
                return m_words.insert(pos, first, last);
        }

        constexpr auto clear() noexcept
                -> void
        {
                m_words.clear();
        }
};

} // namespace test

#endif // TEST_MINIMAL_WORDS_HPP
