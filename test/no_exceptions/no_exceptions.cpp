//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits.hpp> // basic_bit_vector, bit_convert, bit_fixed_set, bit_inplace_vector, bit_set, bit_vector
#include <array>         // array
#include <csignal>       // SIGABRT, signal
#include <cstdint>       // uint8_t, uint64_t
#include <cstdlib>       // _Exit, _set_abort_behavior, EXIT_FAILURE, EXIT_SUCCESS
#include <string_view>   // string_view
#include <utility>       // pair
#include <vector>        // vector

namespace {

// The abort a refused request ends in is this program's success, and the only way out of the handler is _Exit.
[[noreturn]] auto on_abort(int /* signal */) -> void
{
        std::_Exit(EXIT_SUCCESS);
}

// Every member that can refuse, asked for what it grants, so a throw or a try left anywhere fails this build.
auto grant_each() -> bool
{
        auto v = xstd::bit_vector(4);
        v.resize(70);
        v.append_range(std::vector<bool>{true, false});
        v.insert_range(v.cbegin() + 1, std::vector<bool>{true});
        auto const w = v;
        v            = w;

        auto i = xstd::bit_inplace_vector<8>(4);
        i.push_back(true);
        i.reserve(8);

        auto f = xstd::bit_fixed_set<8>{1UZ, 2UZ};
        f.insert(7UZ);

        auto s = xstd::bit_set{1UZ, 100UZ};
        s.insert(5UZ);

        auto const a = xstd::bit_convert<xstd::bit_fixed_set<8>>(xstd::bit_set{1UZ, 7UZ});
        auto const b = xstd::bit_convert<std::uint64_t>(f);
        return static_cast<bool>(v.at(1)) and i.size() == 5UZ and f.contains(7UZ) and s.size() == 3UZ and a.contains(7UZ) and b != 0U;
}

// Each asks one member for what it refuses, which under -fno-exceptions writes the message and aborts.
using refusal = void (*)();

constexpr auto refusals = std::array<std::pair<std::string_view, refusal>, 6>{
        {
                {"at", [] -> void { auto const v = xstd::bit_vector(4); static_cast<void>(static_cast<bool>(v.at(4))); }},
                {"length", [] -> void { auto v = xstd::basic_bit_vector<std::vector<std::uint8_t>>(); v.resize(v.max_size() + 1UZ); }},
                {"capacity", [] -> void { auto v = xstd::bit_inplace_vector<8>(8); v.push_back(true); }},
                {"key", [] -> void { auto f = xstd::bit_fixed_set<8>(); f.insert(8UZ); }},
                {"lost_positions", [] -> void { static_cast<void>(xstd::bit_convert<xstd::bit_fixed_set<8>>(xstd::bit_set{9UZ})); }},
                {"lost_keys", [] -> void { static_cast<void>(xstd::bit_convert<xstd::bit_inplace_set<8>>(xstd::bit_set{9UZ})); }},
        },
};

} // namespace

// With no argument, every member grants its request; with one, the refusal it names, which returns only unrefused.
auto main(int argc, char* argv[]) -> int
{
        if (argc == 1) {
                return grant_each() ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        if (argc != 2) {
                return EXIT_FAILURE;
        }
#ifdef _MSC_VER
        // The debug CRT shows a dialog on abort before raising SIGABRT, which an unattended run waits on forever.
        static_cast<void>(_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT));
#endif
        auto const name = std::string_view(argv[1]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto const& [key, run] : refusals) {
                if (key == name) {
                        static_cast<void>(std::signal(SIGABRT, on_abort));
                        run();
                }
        }
        return EXIT_FAILURE;
}
