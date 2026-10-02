//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // fn_swap_reference, mem_reference_assign, mem_reference_bool, mem_reference_complement, mem_reference_copy, mem_reference_destroy, mem_reference_flip
#include <test/dynamic.hpp>           // dynamic
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, positions, positions_with_singletons
#include <test/spec/input.hpp>        // context
#include <test/spec/rejection.hpp>    // has_append, has_capacity, has_clear, has_empty, has_pop_back, has_push_back, has_reserve, has_resize, has_shrink_to_fit
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Utilities)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(TemplateBitset)
BOOST_AUTO_TEST_SUITE(General)

using namespace test::bitset;
using test::spec::context;
namespace inputs = test::spec::bitset::inputs;

namespace {

// The members a run-time width grows and shrinks by, all of them boost::dynamic_bitset's: a static width has none.
template<class X>
auto check_growth()
        -> void
{
        constexpr auto grows = test::dynamic<X>;
        static_assert(test::spec::has_resize<X> == grows);        // [template.bitset.general]/1
        static_assert(test::spec::has_push_back<X> == grows);     // [template.bitset.general]/1
        static_assert(test::spec::has_pop_back<X> == grows);      // [template.bitset.general]/1
        static_assert(test::spec::has_append<X> == grows);        // [template.bitset.general]/1
        static_assert(test::spec::has_clear<X> == grows);         // [template.bitset.general]/1
        static_assert(test::spec::has_empty<X> == grows);         // [template.bitset.general]/1
        static_assert(test::spec::has_reserve<X> == grows);       // [template.bitset.general]/1
        static_assert(test::spec::has_capacity<X> == grows);      // [template.bitset.general]/1
        static_assert(test::spec::has_shrink_to_fit<X> == grows); // [template.bitset.general]/1
}

} // namespace

// [template.bitset.general]/1: template<size_t N> class bitset;
BOOST_AUTO_TEST_CASE(Bitset)
{
        // A fixed number of bits, N, which a run-time width extends as boost::dynamic_bitset does.
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void { check_growth<T>(); });
        BOOST_CHECK(true);
}

// [template.bitset.general]/4: constexpr reference::reference(const reference& x) noexcept;
BOOST_AUTO_TEST_CASE(ReferenceCopy)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        if (pos < a.size()) {
                                mem_reference_copy()(a, pos);
                        }
                }
        });
}

// [template.bitset.general]/5: constexpr reference::~reference();
BOOST_AUTO_TEST_CASE(ReferenceDestroy)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        if (pos < a.size()) {
                                mem_reference_destroy()(a, pos);
                        }
                }
        });
}

// [template.bitset.general]/6-7: constexpr reference& reference::operator=(bool x) noexcept; and two more
BOOST_AUTO_TEST_CASE(ReferenceAssign)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // The source position mirrors the target, so a singleton meets every other position once.
                for (auto const [from, a, pos] : inputs::positions_with_singletons<T>()) {
                        auto const on_failure = context(from, a, pos);
                        if (pos < a.size()) {
                                mem_reference_assign()(a, pos, a.size() - 1UZ - pos);
                        }
                }
        });
}

// [template.bitset.general]/8: constexpr reference::operator bool() const noexcept;
BOOST_AUTO_TEST_CASE(ReferenceBool)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        if (pos < a.size()) {
                                mem_reference_bool()(a, pos);
                        }
                }
        });
}

// [template.bitset.general]/9: constexpr bool reference::operator~() const noexcept;
BOOST_AUTO_TEST_CASE(ReferenceComplement)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        if (pos < a.size()) {
                                mem_reference_complement()(a, pos);
                        }
                }
        });
}

// [template.bitset.general]/10: constexpr void swap(reference x, reference y) noexcept; and the two with a bool&
BOOST_AUTO_TEST_CASE(ReferenceSwap)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                // A proxy without the hidden-friend swaps, as libstdc++'s and boost's are, passes vacuously.
                for (auto const [from, a, pos] : inputs::positions_with_singletons<T>()) {
                        auto const on_failure = context(from, a, pos);
                        if (pos < a.size()) {
                                fn_swap_reference()(a, pos, a.size() - 1UZ - pos);
                        }
                }
        });
}

// [template.bitset.general]/11-12: constexpr reference& reference::flip() noexcept;
BOOST_AUTO_TEST_CASE(ReferenceFlip)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        if (pos < a.size()) {
                                mem_reference_flip()(a, pos);
                        }
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
