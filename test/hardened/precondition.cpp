//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set/bit_set.hpp>           // bit_set
#include <xstd/bits/bit_span/bit_span.hpp>         // bit_span
#include <xstd/bits/bit_valarray/bit_valarray.hpp> // bit_valarray
#include <xstd/bits/bit_vector.hpp>                // bit_vector
#include <array>                                   // array
#include <csignal>                                 // SIGABRT, signal
#include <cstdlib>                                 // _Exit, _set_abort_behavior, EXIT_FAILURE, EXIT_SUCCESS
#include <string_view>                             // string_view
#include <utility>                                 // pair

namespace {

// The abort a broken precondition ends in is this program's success, and the only way out of the handler is _Exit.
[[noreturn]] auto on_abort(int /* signal */) -> void
{
        std::_Exit(EXIT_SUCCESS);
}

// Each breaks one precondition the hardened standard libraries check on their own containers.
using violation = void (*)();

constexpr auto violations = std::array<std::pair<std::string_view, violation>, 12>{
        {
                {"subscript", [] -> void { auto const v = xstd::bit_vector(4); static_cast<void>(static_cast<bool>(v[4])); }},
                {"front", [] -> void { auto const v = xstd::bit_vector(); static_cast<void>(static_cast<bool>(v.front())); }},
                {"back", [] -> void { auto const v = xstd::bit_vector(); static_cast<void>(static_cast<bool>(v.back())); }},
                {"pop_back", [] -> void { auto v = xstd::bit_vector(); v.pop_back(); }},
                {"erase_end", [] -> void { auto v = xstd::bit_vector(4); v.erase(v.cend()); }},
                {"erase_reversed", [] -> void { auto v = xstd::bit_vector(4); v.erase(v.cbegin() + 3, v.cbegin() + 1); }},
                {"insert_past_end", [] -> void { auto v = xstd::bit_vector(4); v.insert(v.cbegin() + 5, true); }},
                {"first", [] -> void { auto v = xstd::bit_vector(4); static_cast<void>(xstd::bit_span(v).first(5)); }},
                {"last", [] -> void { auto v = xstd::bit_vector(4); static_cast<void>(xstd::bit_span(v).last(5)); }},
                {"subspan", [] -> void { auto v = xstd::bit_vector(4); static_cast<void>(xstd::bit_span(v).subspan(2, 3)); }},
                {"valarray_subscript", [] -> void { auto const v = xstd::bit_valarray(4); static_cast<void>(v[4]); }},
                {"set_erase_end", [] -> void { auto s = xstd::bit_set({1UZ}); s.erase(s.end()); }},
        },
};

} // namespace

// The violation named by the one argument, which returns only where no check fired.
auto main(int argc, char* argv[]) -> int
{
        if (argc != 2) {
                return EXIT_FAILURE;
        }
#ifdef _MSC_VER
        // The debug CRT shows a dialog on abort before raising SIGABRT, which an unattended run waits on forever.
        static_cast<void>(_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT));
#endif
        auto const name = std::string_view(argv[1]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto const& [key, run] : violations) {
                if (key == name) {
                        static_cast<void>(std::signal(SIGABRT, on_abort));
                        run();
                }
        }
        return EXIT_FAILURE;
}
