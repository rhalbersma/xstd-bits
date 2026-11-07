//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/uint128.hpp>                         // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_algorithm/detail/blit.hpp>  // binary_table, unary_table
#include <xstd/bits/bit_iterator/detail/select.hpp> // masked_block
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/bit_stream.hpp>          // shr_up_to_width
#include <xstd/bits/detail/from_blocks.hpp>         // from_blocks
#include <xstd/bits/detail/shift.hpp>               // partial_block_mask, shl, shr
#include <xstd/ints/limits.hpp>                     // numeric_limits
#include <algorithm>                                // max
#include <array>                                    // array
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t, uint16_t, uint32_t, uint64_t
#include <iostream>                                 // cin, cout
#include <string>                                   // stoi, string
#include <string_view>                              // string_view

namespace {

template<class Block>
[[nodiscard]] auto parse(std::string const& text)
        -> Block
{
        auto value = Block{};
        for (auto const c : text) {
                auto const digit = static_cast<unsigned>(c <= '9' ? c - '0' : c - 'a' + 10);
                value            = static_cast<Block>(static_cast<Block>(value << 4) | static_cast<Block>(digit));
        }
        return value;
}

template<class Block>
auto print(Block value)
        -> void
{
        auto text = std::string();
        do {
                text.insert(text.begin(), "0123456789abcdef"[static_cast<unsigned>(value & Block{15})]);
                value = static_cast<Block>(value >> 4);
        } while (value != Block{});
        std::cout << text << ' ';
}

// Each kernel on each line of four hexadecimal blocks and two counts, the counts reduced to what the kernel admits.
template<class Block>
auto run()
        -> void
{
        namespace detail        = xstd::bits::detail;
        constexpr auto width    = static_cast<std::size_t>(xstd::numeric_limits<Block>::digits);
        constexpr auto promoted = std::max(width, 32UZ);
        using storage           = detail::bit_block_container<std::array<Block, 2>, 2 * width>;
        for (std::string a, b, c, d, n_text, m_text; std::cin >> a >> b >> c >> d >> n_text >> m_text;) {
                auto const x     = parse<Block>(a);
                auto const y     = parse<Block>(b);
                auto const value = parse<Block>(c);
                auto const mask  = parse<Block>(d);
                auto const n     = parse<std::size_t>(n_text);
                auto const m     = parse<std::size_t>(m_text);

                print(detail::shl(x, n % promoted));
                print(detail::shr(x, n % promoted));
                print(detail::partial_block_mask<Block>(n % (width + 1)));
                print(detail::shr_up_to_width(x, n % (width + 1)));

                auto const p    = n % (2 * width);
                auto const bits = storage(xstd::from_blocks, std::array<Block, 2>{x, y});
                print(bits.block_at(p));
                auto written     = bits;
                auto const inner = p <= width ? mask : static_cast<Block>(mask & detail::partial_block_mask<Block>((2 * width) - p));
                written.block_at(p, value, inner);
                print(written[0]);
                print(written[1]);

                auto const first = p;
                auto const last  = first + 1 + (m % ((2 * width) - first));
                print(detail::masked_block(bits, first / width, first, last));
                print(detail::masked_block(bits, (last - 1) / width, first, last));

                auto unary  = [index = m % 4](bool v) -> bool { return ((index >> (v ? 1 : 0)) & 1UZ) != 0; };
                auto binary = [index = m % 16](bool u, bool v) -> bool { return ((index >> ((u ? 2 : 0) + (v ? 1 : 0))) & 1UZ) != 0; };
                print(detail::unary_table<Block>(unary)(x));
                print(detail::binary_table<Block>(binary)(x, y));
                std::cout << '\n';
        }
}

} // namespace

// The compiled block kernels at the width asked, on each line read, for the models of them to be held against.
auto main(int argc, char* argv[]) -> int
{
        if (argc != 2) {
                return 2;
        }
        auto const arg = std::string_view(argv[1]);
        if (arg == "--widths") {
#ifdef TEST_HAS_UINT128
                std::cout << "8 16 32 64 128\n";
#else
                std::cout << "8 16 32 64\n";
#endif
                return 0;
        }
        switch (std::stoi(std::string(arg))) {
                case 8:
                        run<std::uint8_t>();
                        break;
                case 16:
                        run<std::uint16_t>();
                        break;
                case 32:
                        run<std::uint32_t>();
                        break;
                case 64:
                        run<std::uint64_t>();
                        break;
#ifdef TEST_HAS_UINT128
                case 128:
                        run<xstd::uint128>();
                        break;
#endif
                default:
                        return 2;
        }
        return 0;
}
