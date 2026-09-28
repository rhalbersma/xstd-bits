//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // fn_swap_reference, mem_reference_assign, mem_reference_bool, mem_reference_complement, mem_reference_copy, mem_reference_destroy, mem_reference_flip
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, positions, positions_with_singletons
#include <test/spec/input.hpp>        // context
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Utilities)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(TemplateBitset)
BOOST_AUTO_TEST_SUITE(General)

using namespace test::bitset;
using test::spec::context;
namespace inputs = test::spec::bitset::inputs;

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
