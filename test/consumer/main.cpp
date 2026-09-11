//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The gate on the interface line. [design.md#the-interface-line]
//
// One xstd include, and every name below is reached through it: the nine containers, the three views, the three
// adaptors, ownership and bit_traits. No <xstd/bits/...> header for any of them, and above all nothing under
// detail/ -- a name that cannot be spelled from here is not interface, and this is where that is enforced rather
// than asserted. ext/ is absent on purpose: it is interface, but the umbrella leaves it out so Boost stays off
// every consumer's path.

#include <xstd/bits.hpp> // bit_array, bit_inplace_set, bit_inplace_vector, bit_set, bit_set_view, bit_span,
                         // bit_static_set, bit_subspan, bit_traits, bit_vector, bitset, bitset_adaptor,
                         // dynamic_bitset, has_bitops, inplace_bitset, ownership, sequence_adaptor, set_adaptor
#include <concepts>      // same_as
#include <cstddef>       // size_t
#include <cstdint>       // uint8_t, uint64_t
#include <version>       // IWYU pragma: keep; __cpp_lib_inplace_vector

namespace consumer {

// A storage of our own, to be adapted through the one extension point the library documents. [design.md#the-trait]
struct word
{
        std::uint64_t bits = 0;
};

}       // namespace consumer

namespace xstd {

// The trait door, taken the way a user takes it: a specialization, no ADL hook, nothing from detail/.
template<>
struct bit_traits<consumer::word>
{
        using bits_type = consumer::word;

        static constexpr std::size_t extent = 64;

        [[nodiscard]] static constexpr auto size(bits_type const&)                  noexcept -> std::size_t { return extent;                          }
        [[nodiscard]] static constexpr auto at  (bits_type const& c, std::size_t n) noexcept -> bool        { return ((c.bits >> n) & 1ULL) != 0ULL;   }

        // The block tier, so the scans read a word at a time rather than a bit. [design.md#detection-by-absence]
        [[nodiscard]] static constexpr auto num_blocks(bits_type const&)                                   noexcept -> std::size_t   { return 1UZ;    }
        [[nodiscard]] static constexpr auto block     (bits_type const& c, std::size_t i [[maybe_unused]]) noexcept -> std::uint64_t { return c.bits; }

        static constexpr auto unchecked_assign(bits_type& c, std::size_t n, bool value) noexcept
                -> void
        {
                auto const mask = static_cast<std::uint64_t>(1ULL << n);
                c.bits = value ? static_cast<std::uint64_t>(c.bits | mask) : static_cast<std::uint64_t>(c.bits & ~mask);
        }

        // A static width cannot grow, so inserting is assigning. [design.md#what-the-trait-reconciles]
        static constexpr auto insert(bits_type& c, std::size_t n)    noexcept -> void { unchecked_assign(c, n, true); }
        static constexpr auto fill  (bits_type& c, bool value)       noexcept -> void { c.bits = value ? ~std::uint64_t{} : std::uint64_t{}; }
};

}       // namespace xstd

namespace consumer {

// The adaptors named without naming the storage they are instantiated over: a pattern match, which is also the
// claim #131 rests on -- the containers and the views are not built on the adaptors, they are the adaptors.
template<class>                                            constexpr bool is_set_adaptor = false;
template<class B, xstd::ownership O, class T>              constexpr bool is_set_adaptor<xstd::set_adaptor<B, O, T>> = true;

template<class>                                            constexpr bool is_sequence_adaptor = false;
template<class B, xstd::ownership O, bool W, class T>      constexpr bool is_sequence_adaptor<xstd::sequence_adaptor<B, O, W, T>> = true;

template<class>                                            constexpr bool is_bitset_adaptor = false;
template<class B, class T>                                 constexpr bool is_bitset_adaptor<xstd::bitset_adaptor<B, T>> = true;

// The set reading: three widths, one adaptor.
static_assert(is_set_adaptor<xstd::bit_static_set<100>>);
static_assert(is_set_adaptor<xstd::basic_bit_static_set<std::uint8_t, 24>>);
static_assert(is_set_adaptor<xstd::bit_set>);
static_assert(is_set_adaptor<xstd::bit_set_view<word>>);

// The sequence reading, the window included.
static_assert(is_sequence_adaptor<xstd::bit_array<64>>);
static_assert(is_sequence_adaptor<xstd::basic_bit_array<std::uint8_t, 24>>);
static_assert(is_sequence_adaptor<xstd::bit_vector>);
static_assert(is_sequence_adaptor<xstd::bit_span<word>>);
static_assert(is_sequence_adaptor<xstd::bit_subspan<word>>);

// The bitset reading, which owns by construction: its storage must speak the whole bitset vocabulary, and only
// the library's own vehicles do -- so our word is here as the negative case rather than as an instantiation.
static_assert(is_bitset_adaptor<xstd::bitset<64>>);
static_assert(is_bitset_adaptor<xstd::basic_bitset<std::uint8_t, 24>>);
static_assert(is_bitset_adaptor<xstd::dynamic_bitset>);
static_assert(xstd::has_bitops<xstd::bitset<64>>);
static_assert(not xstd::has_bitops<word>);

// ownership is interface because you cannot name an adaptor without it.
static_assert(xstd::owns(xstd::ownership::owns));
static_assert(not xstd::owns(xstd::ownership::refers));
static_assert(std::same_as<xstd::bit_set_view<word>, xstd::set_adaptor<word, xstd::ownership::refers>>);

// The trait door answers about our storage, through the concepts the library publishes.
static_assert(xstd::bit_storage<xstd::bit_traits<word>, word>);
static_assert(xstd::static_bit_extent<xstd::bit_traits<word>, word>);
static_assert(xstd::block_readable<xstd::bit_traits<word>, word>);

#ifdef __cpp_lib_inplace_vector

// The inplace column, present only where its storage is. [design.md#the-inplace-column]
static_assert(is_set_adaptor<xstd::bit_inplace_set<100>>);
static_assert(is_sequence_adaptor<xstd::bit_inplace_vector<100>>);
static_assert(is_bitset_adaptor<xstd::inplace_bitset<100>>);

#endif

}       // namespace consumer

int main()
{
        auto failures = 0;
        auto const check = [&failures](bool ok) noexcept { failures += ok ? 0 : 1; };

        // The set reading over storage the container owns.
        auto set = xstd::bit_static_set<100>();
        set.insert(1);
        set.insert(2);
        set.insert(3);
        check(set.size() == 3);
        check(set.contains(2));
        check(*set.begin() == 1);

        auto grown = xstd::bit_set();
        grown.insert(64);
        check(grown.contains(64));

        // The sequence reading, and the bitset reading.
        auto array = xstd::bit_array<64>();
        array[7] = true;
        check(array.count() == 1);

        auto bits = xstd::bitset<64>();
        bits.set(5);
        check(bits.test(5) and bits.count() == 1);

        auto dynamic = xstd::dynamic_bitset(64);
        dynamic.set(5);
        check(dynamic.test(5) and dynamic.count() == 1);

        auto vector = xstd::bit_vector(64);
        vector[63] = true;
        check(vector.size() == 64 and vector.count() == 1);

#ifdef __cpp_lib_inplace_vector

        auto inplace = xstd::bit_inplace_set<100>();
        inplace.insert(99);
        check(inplace.contains(99));

#endif

        // The views over our own storage: the trait door and the three view names, end to end.
        auto storage = consumer::word{};
        auto view = xstd::bit_set_view(storage);
        view.insert(9);
        view.insert(40);
        check(storage.bits == ((1ULL << 9) | (1ULL << 40)));
        check(view.size() == 2);

        auto span = xstd::bit_span(storage);
        check(span.count() == 2);
        check(span[9] and not span[10]);

        xstd::bit_subspan<consumer::word> const window = span.subspan(8, 8);
        check(window.size() == 8);
        check(window.count() == 1);

        // Const storage reaches a read-only view, and the const is part of the type. [design.md#read-only-set-proxy]
        auto const& frozen = storage;
        auto const reader = xstd::bit_set_view(frozen);
        check(reader.size() == 2);

        return failures;
}
