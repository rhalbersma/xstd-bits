//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>      // bit_array
#include <xstd/bits/bit_set.hpp>        // basic_bit_set, bit_set
#include <xstd/bits/bit_vector.hpp>     // basic_bit_vector, bit_vector
#include <xstd/bits/dynamic_bitset.hpp> // basic_dynamic_bitset, dynamic_bitset
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <cstddef>                      // size_t
#include <initializer_list>             // initializer_list
#include <memory>                       // allocator, uses_allocator_v
#include <memory_resource>              // monotonic_buffer_resource, polymorphic_allocator
#include <type_traits>                  // is_constructible_v, is_nothrow_constructible_v
#include <utility>                      // move
#include <vector>                       // pmr::vector, vector

BOOST_AUTO_TEST_SUITE(Allocators)

namespace {

using pmr_bit_set = xstd::basic_bit_set<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
using pmr_bit_vector = xstd::basic_bit_vector<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;
using pmr_dynamic_bitset = xstd::basic_dynamic_bitset<std::size_t, std::pmr::polymorphic_allocator<std::size_t>>;

// Declared only, for the concept below to call in an unevaluated operand.
template<class C>
[[maybe_unused]] auto accept(C)
        -> void;

// Copy-list-initialization from the braced list: what an explicit constructor refuses.
template<class C, class... Args>
concept list_converts_from = requires (Args... args) { accept<C>({args...}); };

} // namespace

// An allocator converts as [container.alloc.reqmts] has it: a memory_resource* makes a polymorphic_allocator.
BOOST_AUTO_TEST_CASE(AMemoryResourceConvertsToThePolymorphicAllocator)
{
        auto mr = std::pmr::monotonic_buffer_resource();

        auto const s = pmr_bit_set({1, 2}, &mr);
        BOOST_CHECK(s.get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(s.size(), 2UZ);

        auto const v = pmr_bit_vector(3, true, &mr);
        BOOST_CHECK(v.get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(v.size(), 3UZ);

        auto const b = pmr_dynamic_bitset(&mr);
        BOOST_CHECK(b.get_allocator().resource() == &mr);
}

// Uses-allocator construction: an allocator-aware container hands each element its own allocator, converted.
BOOST_AUTO_TEST_CASE(AnAllocatorAwareContainerPassesItsAllocatorOn)
{
        static_assert(std::uses_allocator_v<pmr_bit_vector, std::pmr::polymorphic_allocator<pmr_bit_vector>>);

        auto mr = std::pmr::monotonic_buffer_resource();
        auto sets = std::pmr::vector<pmr_bit_set>(&mr);
        auto vectors = std::pmr::vector<pmr_bit_vector>(&mr);
        auto bitsets = std::pmr::vector<pmr_dynamic_bitset>(&mr);

        sets.emplace_back();
        vectors.emplace_back(3);
        bitsets.emplace_back();

        BOOST_CHECK(sets.back().get_allocator().resource() == &mr);
        BOOST_CHECK(vectors.back().get_allocator().resource() == &mr);
        BOOST_CHECK_EQUAL(vectors.back().size(), 3UZ);
        BOOST_CHECK(bitsets.back().get_allocator().resource() == &mr);
}

// A rebound std::allocator converts, and a braced {} is a value-initialized allocator, as for std::vector<bool>.
BOOST_AUTO_TEST_CASE(AConvertibleOrEmptyAllocatorArgumentIsTaken)
{
        static_assert(std::is_constructible_v<std::vector<bool>, std::size_t, std::allocator<int>>);
        static_assert(std::is_constructible_v<xstd::bit_vector, std::size_t, std::allocator<int>>);
        static_assert(std::is_constructible_v<xstd::bit_set, std::allocator<int>>);
        static_assert(std::is_constructible_v<xstd::dynamic_bitset, std::allocator<int>>);

        auto const v = xstd::bit_vector(3, true, {});
        BOOST_CHECK_EQUAL(v.size(), 3UZ);
}

// std::vector<bool>'s and boost's count constructors are explicit with the allocator as without it.
BOOST_AUTO_TEST_CASE(TheCountConstructorsAreExplicit)
{
        static_assert(not list_converts_from<std::vector<bool>, std::size_t, std::allocator<bool>>);
        static_assert(not list_converts_from<xstd::bit_vector, std::size_t, std::allocator<std::size_t>>);
        static_assert(not list_converts_from<xstd::dynamic_bitset, std::size_t, unsigned long long, std::allocator<std::size_t>>);
        BOOST_CHECK(true);
}

// The allocator-only constructors cannot throw, as [vector.bool.pspc] has it; a width in the type takes no allocator.
BOOST_AUTO_TEST_CASE(OnlyARunTimeWidthTakesAnAllocator)
{
        static_assert(std::is_nothrow_constructible_v<std::vector<bool>, std::allocator<bool> const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::bit_vector, std::allocator<std::size_t> const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::bit_set, std::allocator<std::size_t> const&>);
        static_assert(std::is_nothrow_constructible_v<xstd::dynamic_bitset, std::allocator<std::size_t> const&>);
        static_assert(not std::is_constructible_v<xstd::bit_array<64>, std::allocator<std::size_t>>);
        static_assert(not std::is_constructible_v<xstd::bit_array<64>, std::initializer_list<bool>, std::allocator<std::size_t>>);
        BOOST_CHECK(true);
}

// [container.alloc.reqmts]'s allocator-extended copy and move, which dynamic_bitset has where boost has not.
BOOST_AUTO_TEST_CASE(ABitsetIsCopiedAndMovedIntoAnotherAllocator)
{
        auto mr = std::pmr::monotonic_buffer_resource();
        auto source = pmr_dynamic_bitset(70, 5ULL);

        auto const copy = pmr_dynamic_bitset(source, &mr);
        BOOST_CHECK(copy == source and copy.get_allocator().resource() == &mr);

        auto const moved = pmr_dynamic_bitset(std::move(source), &mr);
        BOOST_CHECK(moved == copy and moved.get_allocator().resource() == &mr);
}

// std::set's comparator arguments are accepted and, std::less having no state, change nothing.
BOOST_AUTO_TEST_CASE(TheComparatorArgumentsAreAcceptedAsStdSetsAre)
{
        auto const comp = xstd::bit_set::key_compare(); // NOLINT(modernize-use-transparent-functors): std::set<std::size_t>::key_compare
        BOOST_CHECK(xstd::bit_set(comp).empty());
        BOOST_CHECK(xstd::bit_set({3, 1}, comp) == xstd::bit_set({1, 3}));
        BOOST_CHECK(xstd::bit_set({3, 1}, comp, std::allocator<std::size_t>()) == xstd::bit_set({1, 3}));
}

BOOST_AUTO_TEST_SUITE_END()
