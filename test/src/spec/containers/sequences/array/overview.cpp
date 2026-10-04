//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/reference.hpp>           // proxy_reference
#include <test/sequence/factory.hpp>    // make_sequence, model_of, stripes
#include <test/sequence/primitives.hpp> // iterates_as_a_constant
#include <test/spec/rejection.hpp>      // has_assign_count, has_capacity, has_clear, has_emplace_back, has_erase_at, has_insert_at, has_pop_back, has_push_back, has_reserve, has_resize, has_shrink_to_fit
#include <test/spec/sequence.hpp>       // array_all
#include <test/structural.hpp>          // structural, value_parameter
#include <xstd/bits/bit_array.hpp>      // basic_bit_array
#include <xstd/bits/bit_vector.hpp>     // bit_vector
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                     // same_as
#include <cstddef>                      // size_t
#include <iterator>                     // reverse_iterator
#include <limits>                       // numeric_limits
#include <ranges>                       // bidirectional_range, contiguous_range, random_access_range
#include <vector>                       // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Array)
BOOST_AUTO_TEST_SUITE(Overview)

using namespace test::sequence;

namespace {

// Where a packed array is structural: at a width with no unused bits for a public block to let a caller set.
template<class T>
inline constexpr bool fills_its_blocks = true;

template<class Block, std::size_t N>
inline constexpr bool fills_its_blocks<xstd::basic_bit_array<Block, N>> = N % static_cast<std::size_t>(std::numeric_limits<Block>::digits) == 0UZ;

// Only the last element set, the one a packed array keeps in the high bit of its last block.
template<class T>
[[nodiscard]] constexpr auto make_last()
        -> T
{
        auto a   = T();
        a.back() = true;
        return a;
}

} // namespace

// [array.overview]/1: template<class T, size_t N> struct array;
BOOST_AUTO_TEST_CASE(Array)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                // A contiguous container, which a proxy relaxes to random access: a bit has no address to lie at.
                static_assert(std::ranges::contiguous_range<T> or (test::proxy_reference<T> and std::ranges::random_access_range<T>)); // [array.overview]/1
                auto const a = make_sequence<T>(T().size(), stripes);
                auto b       = a;
                b            = T();
                BOOST_CHECK_EQUAL(b.size(), a.size()); // [array.overview]/1
        });
}

// [array.overview]/2: list-initialization with up to N elements
BOOST_AUTO_TEST_CASE(ListInitialization)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                // What is listed leads, each in its place, and every position after it is value-initialized.
                constexpr auto N = T().size();
                auto const none  = std::vector<bool>(N, false);
                BOOST_CHECK(model_of(T{}) == none); // [array.overview]/2
                if constexpr (N >= 1UZ) {
                        auto m = none;
                        m[0]   = true;
                        BOOST_CHECK(model_of(T{true}) == m);
                }
                if constexpr (N >= 3UZ) {
                        static_assert(requires (bool b) { T{b, b}; });
                        auto m = none;
                        m[0]   = true;
                        m[2]   = true;
                        BOOST_CHECK(model_of(T{true, false, true}) == m);
                }
        });
}

// [array.overview]/3: a container and a reversible container, but not empty when default constructed
BOOST_AUTO_TEST_CASE(ContainerRequirements)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                static_assert(std::ranges::bidirectional_range<T> and std::same_as<typename T::reverse_iterator, std::reverse_iterator<typename T::iterator>>); // [array.overview]/3
                // Only some of a sequence container's requirements: none that changes the size.
                static_assert(not test::spec::has_resize<T>);        // [array.overview]/3
                static_assert(not test::spec::has_push_back<T>);     // [array.overview]/3
                static_assert(not test::spec::has_emplace_back<T>);  // [array.overview]/3
                static_assert(not test::spec::has_pop_back<T>);      // [array.overview]/3
                static_assert(not test::spec::has_insert_at<T>);     // [array.overview]/3
                static_assert(not test::spec::has_erase_at<T>);      // [array.overview]/3
                static_assert(not test::spec::has_assign_count<T>);  // [array.overview]/3
                static_assert(not test::spec::has_clear<T>);         // [array.overview]/3
                static_assert(not test::spec::has_reserve<T>);       // [array.overview]/3
                static_assert(not test::spec::has_capacity<T>);      // [array.overview]/3
                static_assert(not test::spec::has_shrink_to_fit<T>); // [array.overview]/3
                auto const u = T();
                BOOST_CHECK_EQUAL(u.size(), T().max_size());   // [array.overview]/3
                BOOST_CHECK_EQUAL(u.empty(), u.size() == 0UZ); // [array.overview]/3
        });
        // A growing column has every one of them, so a misspelled member cannot pass the checks above.
        using G = xstd::bit_vector;
        static_assert(test::spec::has_resize<G> and test::spec::has_push_back<G> and test::spec::has_emplace_back<G> and test::spec::has_pop_back<G>);
        static_assert(test::spec::has_insert_at<G> and test::spec::has_erase_at<G> and test::spec::has_assign_count<G> and test::spec::has_clear<G>);
        static_assert(test::spec::has_reserve<G> and test::spec::has_capacity<G> and test::spec::has_shrink_to_fit<G>);
}

// [array.overview]/4: array<T, N> is a structural type if T is, its values template-argument-equivalent by element
BOOST_AUTO_TEST_CASE(StructuralType)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                static_assert(test::structural<T> == fills_its_blocks<T>); // [array.overview]/4
                constexpr auto N = T().size();
                if constexpr (fills_its_blocks<T> and N >= 1UZ) {
                        static_assert(std::same_as<test::value_parameter<T{}>, test::value_parameter<T{false}>>);                  // [array.overview]/4
                        static_assert(std::same_as<test::value_parameter<make_last<T>()>, test::value_parameter<make_last<T>()>>); // [array.overview]/4
                        static_assert(not std::same_as<test::value_parameter<T{}>, test::value_parameter<make_last<T>()>>);        // [array.overview]/4
                }
                if constexpr (fills_its_blocks<T> and N >= 2UZ) {
                        static_assert(std::same_as<test::value_parameter<T{true}>, test::value_parameter<T{true, false}>>);     // [array.overview]/4
                        static_assert(not std::same_as<test::value_parameter<T{true}>, test::value_parameter<T{false, true}>>); // [array.overview]/4
                }
        });
}

// [array.overview]/5: iterator and const_iterator are constexpr iterators
BOOST_AUTO_TEST_CASE(ConstexprIterators)
{
        test::for_each_type<test::spec::sequence::array_all>([]<class T> -> void {
                static_assert(iterates_as_a_constant<T>()); // [array.overview]/5
                BOOST_CHECK(iterates_as_a_constant<T>());   // [array.overview]/5
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
