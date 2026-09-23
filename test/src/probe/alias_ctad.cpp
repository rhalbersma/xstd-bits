//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                    // array
#include <concepts>                 // same_as, unsigned_integral
#include <cstddef>                  // size_t
#include <cstdint>                  // uint64_t
#include <limits>                   // numeric_limits
#include <memory>                   // allocator
#include <span>                     // dynamic_extent
#include <tuple>                    // tuple_size_v
#include <utility>                  // declval
#include <vector>                   // vector

// Which shapes of deduction through an alias template (P1814) MSVC 17 accepts, one case per static_assert.
namespace probe {

struct from_bits_t
{
        explicit from_bits_t() = default;
};

inline constexpr auto from_bits = from_bits_t();

template<std::unsigned_integral Block>
inline constexpr std::size_t digits = static_cast<std::size_t>(std::numeric_limits<Block>::digits);

template<std::unsigned_integral Block>
[[nodiscard]] constexpr auto blocks_for(std::size_t n) noexcept
        -> std::size_t
{
        return (n + digits<Block> - 1) / digits<Block>;
}

template<class C>
inline constexpr std::size_t default_width = std::dynamic_extent;

template<std::unsigned_integral Block, std::size_t K>
inline constexpr std::size_t default_width<std::array<Block, K>> = digits<Block> * K;

// A public adaptor: a storage and a width, the width defaulted from the storage.
template<class C, std::size_t N = default_width<C>>
class adaptor
{
public:
        adaptor() = default;
        explicit adaptor(C const&);
        template<std::unsigned_integral Block>
        adaptor(from_bits_t, Block);
        adaptor(from_bits_t, C const&);
};

template<class C>
explicit adaptor(C const&) -> adaptor<C>;

// The return types compute the block count from the width, as the aliases do, or no alias can deduce through them.
template<std::unsigned_integral Block>
adaptor(from_bits_t, Block) -> adaptor<std::array<Block, blocks_for<Block>(digits<Block>)>, digits<Block>>;

template<std::unsigned_integral Block, std::size_t K>
adaptor(from_bits_t, std::array<Block, K>) -> adaptor<std::array<Block, blocks_for<Block>(digits<Block>* K)>, digits<Block> * K>;

// The owner shapes: an argument computed from the others, and a defaulted allocator.
template<std::unsigned_integral Block, std::size_t N>
using basic_bit_array = adaptor<std::array<Block, blocks_for<Block>(N)>, N>;

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

template<std::unsigned_integral Block, class Allocator = std::allocator<Block>>
using basic_bit_vector = adaptor<std::vector<Block, Allocator>>;

// The view shape MSVC 17 was seen to reject: one pinned non-type argument beside a defaulted constrained one.
enum class storage : bool { owned,
                            borrowed,
};

template<class Bits, storage S>
struct self
{};

template<class Bits, storage S, std::same_as<self<Bits, S>> Derived = self<Bits, S>>
class set_adaptor
{
public:
        explicit set_adaptor(Bits&);
};

template<class Bits>
set_adaptor(Bits&) -> set_adaptor<Bits, storage::borrowed>;

template<class Bits>
using bit_set_view = set_adaptor<Bits, storage::borrowed>;

template<class T>
concept array_of_words = std::same_as<T, std::array<typename T::value_type, std::tuple_size_v<T>>>;

// The alias as it was written, its parameter constrained.
template<array_of_words Bits>
using constrained_bit_set_view = set_adaptor<Bits, storage::borrowed>;

using word = std::uint64_t;
using words = std::array<word, 2>;

// 1. The adaptor's own guides, reached by its own name.
static_assert(std::same_as<decltype(adaptor(from_bits, word())), adaptor<std::array<word, 1>, 64>>);
static_assert(std::same_as<decltype(adaptor(from_bits, words())), adaptor<words, 128>>);

// 2. An owner alias whose block count is computed from the width.
static_assert(std::same_as<decltype(basic_bit_array(from_bits, word())), basic_bit_array<word, 64>>);
static_assert(std::same_as<decltype(basic_bit_array(from_bits, words())), basic_bit_array<word, 128>>);

// 3. An alias of that alias, with the block type pinned.
static_assert(std::same_as<decltype(bit_array(from_bits, std::size_t())), bit_array<64>>);

// 4. An owner alias with a defaulted allocator, deduced from the storage it adopts.
static_assert(std::same_as<decltype(basic_bit_vector(std::vector<word>())), basic_bit_vector<word>>);

// 5. The view shape, unconstrained and then constrained, as the controls for the diagnostic seen before.
static_assert(std::same_as<decltype(bit_set_view(std::declval<words&>())), bit_set_view<words>>);
static_assert(std::same_as<decltype(constrained_bit_set_view(std::declval<words&>())), constrained_bit_set_view<words>>);

} // namespace probe

BOOST_AUTO_TEST_SUITE(AliasCtad)

BOOST_AUTO_TEST_CASE(EveryShapeCompiles)
{
        BOOST_CHECK(probe::blocks_for<probe::word>(65) == 2);
}

BOOST_AUTO_TEST_SUITE_END()
