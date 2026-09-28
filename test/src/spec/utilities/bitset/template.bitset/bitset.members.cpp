//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/primitives.hpp> // mem_all, mem_any, mem_at, mem_bit_and_assign, mem_bit_not, mem_bit_or_assign, mem_bit_xor_assign, mem_count, mem_equal_to, mem_flip, mem_none, mem_reset, mem_set, mem_shift_left, mem_shift_left_assign, mem_shift_right, mem_shift_right_assign, mem_size, mem_test, mem_to_string, mem_to_ullong, mem_to_ulong
#include <test/for_each_type.hpp>     // for_each_type
#include <test/spec/bitset.hpp>       // all, bitsets, pairs, positions, positions_with_singletons
#include <test/spec/input.hpp>        // context, on_copy
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Utilities)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(TemplateBitset)
BOOST_AUTO_TEST_SUITE(BitsetMembers)

using namespace test::bitset;
using test::spec::context;
using test::spec::on_copy;
namespace inputs = test::spec::bitset::inputs;

// [bitset.members]/1-2: constexpr bitset& operator&=(const bitset& rhs) noexcept;
BOOST_AUTO_TEST_CASE(AndAssign)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        on_copy(mem_bit_and_assign(), a, b);
                }
        });
}

// [bitset.members]/3-4: constexpr bitset& operator|=(const bitset& rhs) noexcept;
BOOST_AUTO_TEST_CASE(OrAssign)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        on_copy(mem_bit_or_assign(), a, b);
                }
        });
}

// [bitset.members]/5-6: constexpr bitset& operator^=(const bitset& rhs) noexcept;
BOOST_AUTO_TEST_CASE(XorAssign)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        on_copy(mem_bit_xor_assign(), a, b);
                }
        });
}

// [bitset.members]/7-8: constexpr bitset& operator<<=(size_t pos) noexcept;
BOOST_AUTO_TEST_CASE(ShiftLeftAssign)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions_with_singletons<T>()) {
                        auto const on_failure = context(from, a, pos);
                        on_copy(mem_shift_left_assign(), a, pos);
                }
        });
}

// [bitset.members]/9-10: constexpr bitset& operator>>=(size_t pos) noexcept;
BOOST_AUTO_TEST_CASE(ShiftRightAssign)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions_with_singletons<T>()) {
                        auto const on_failure = context(from, a, pos);
                        on_copy(mem_shift_right_assign(), a, pos);
                }
        });
}

// [bitset.members]/11: constexpr bitset operator<<(size_t pos) const noexcept;
BOOST_AUTO_TEST_CASE(ShiftLeft)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions_with_singletons<T>()) {
                        auto const on_failure = context(from, a, pos);
                        mem_shift_left()(a, pos);
                }
        });
}

// [bitset.members]/12: constexpr bitset operator>>(size_t pos) const noexcept;
BOOST_AUTO_TEST_CASE(ShiftRight)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions_with_singletons<T>()) {
                        auto const on_failure = context(from, a, pos);
                        mem_shift_right()(a, pos);
                }
        });
}

// [bitset.members]/13-14: constexpr bitset& set() noexcept;
BOOST_AUTO_TEST_CASE(Set)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        on_copy(mem_set(), a);
                }
        });
}

// [bitset.members]/15-17: constexpr bitset& set(size_t pos, bool val = true);
BOOST_AUTO_TEST_CASE(SetPos)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        on_copy(mem_set(), a, pos);
                        on_copy(mem_set(), a, pos, true);
                        on_copy(mem_set(), a, pos, false);
                }
        });
}

// [bitset.members]/18-19: constexpr bitset& reset() noexcept;
BOOST_AUTO_TEST_CASE(Reset)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        on_copy(mem_reset(), a);
                }
        });
}

// [bitset.members]/20-22: constexpr bitset& reset(size_t pos);
BOOST_AUTO_TEST_CASE(ResetPos)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        on_copy(mem_reset(), a, pos);
                }
        });
}

// [bitset.members]/23-24: constexpr bitset operator~() const noexcept;
BOOST_AUTO_TEST_CASE(Complement)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_bit_not()(a);
                }
        });
}

// [bitset.members]/25-26: constexpr bitset& flip() noexcept;
BOOST_AUTO_TEST_CASE(Flip)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        on_copy(mem_flip(), a);
                }
        });
}

// [bitset.members]/27-29: constexpr bitset& flip(size_t pos);
BOOST_AUTO_TEST_CASE(FlipPos)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        on_copy(mem_flip(), a, pos);
                }
        });
}

// [bitset.members]/30-32: constexpr bool operator[](size_t pos) const;
BOOST_AUTO_TEST_CASE(Subscript)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        if (pos < a.size()) {
                                mem_at()(a, pos);
                        }
                }
        });
}

// [bitset.members]/37-38: constexpr unsigned long to_ulong() const;
BOOST_AUTO_TEST_CASE(ToUlong)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_to_ulong()(a);
                }
        });
}

// [bitset.members]/39-40: constexpr unsigned long long to_ullong() const;
BOOST_AUTO_TEST_CASE(ToUllong)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_to_ullong()(a);
                }
        });
}

// [bitset.members]/41-42: constexpr basic_string<charT, traits, Allocator> to_string(charT zero, charT one) const;
BOOST_AUTO_TEST_CASE(ToString)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_to_string()(a);
                }
        });
}

// [bitset.members]/43: constexpr size_t count() const noexcept;
BOOST_AUTO_TEST_CASE(Count)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_count()(a);
                }
        });
}

// [bitset.members]/44: constexpr size_t size() const noexcept;
BOOST_AUTO_TEST_CASE(Size)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_size()(a);
                }
        });
}

// [bitset.members]/45: constexpr bool operator==(const bitset& rhs) const noexcept;
BOOST_AUTO_TEST_CASE(EqualTo)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_equal_to()(a, b);
                }
        });
}

// [bitset.members]/46-47: constexpr bool test(size_t pos) const;
BOOST_AUTO_TEST_CASE(Test)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a, pos] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, pos);
                        mem_test()(a, pos);
                }
        });
}

// [bitset.members]/48: constexpr bool all() const noexcept;
BOOST_AUTO_TEST_CASE(All)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_all()(a);
                }
        });
}

// [bitset.members]/49: constexpr bool any() const noexcept;
BOOST_AUTO_TEST_CASE(Any)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_any()(a);
                }
        });
}

// [bitset.members]/50: constexpr bool none() const noexcept;
BOOST_AUTO_TEST_CASE(None)
{
        test::for_each_type<test::spec::bitset::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::bitsets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_none()(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
