//          Copyright Rein Halbersma 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_REFUSAL_MESSAGE_HPP
#define XSTD_BITS_DETAIL_REFUSAL_MESSAGE_HPP

#include <array>           // array
#include <charconv>        // to_chars
#include <cstddef>         // size_t
#include <limits>          // numeric_limits
#include <source_location> // source_location
#include <string>          // string
#include <string_view>     // string_view

namespace xstd::bits::detail {

[[nodiscard]] constexpr auto decimal(std::size_t n)
        -> std::string
{
        auto digits     = std::array<char, std::numeric_limits<std::size_t>::digits10 + 1>{};
        auto const last = std::to_chars(digits.data(), digits.data() + digits.size(), n).ptr;
        return std::string(digits.data(), last);
}

// "file:line:column: exception: ‘function‘: complaint [n relation bound]", built without <format> and all it includes.
[[nodiscard]] constexpr auto refusal_message(std::source_location const& loc, std::string_view complaint, std::size_t n, std::string_view relation, std::size_t bound)
        -> std::string
{
        auto message = std::string(loc.file_name());
        message += ':';
        message += decimal(loc.line());
        message += ':';
        message += decimal(loc.column());
        message += ": exception: ‘";
        message += loc.function_name();
        message += "‘: ";
        message += complaint;
        message += " [";
        message += decimal(n);
        message += ' ';
        message += relation;
        message += ' ';
        message += decimal(bound);
        message += ']';
        return message;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_REFUSAL_MESSAGE_HPP
