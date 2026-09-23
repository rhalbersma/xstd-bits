//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set.hpp>                         // bit_set
#include <xstd/bits/bit_vector.hpp>                      // bit_vector
#include <xstd/bits/borrowed_bits.hpp>                   // borrow_bits, borrowed_bits
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t, owned_storage, owner_reading, reading, storage
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor
#include <xstd/bits/detail/set_adaptor.hpp>              // set_adaptor
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                                      // same_as
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint64_t
#include <type_traits>                                   // conditional_t, is_const_v, remove_const_t
#include <utility>                                       // declval

// Real: the views as the aliases they once were, with the two guides they once had on the detail adaptors.
namespace xstd::detail::bits {

template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
set_adaptor(Bits&) -> set_adaptor<Bits, storage::borrowed>;

template<owner_reading<reading::set> Owner>
set_adaptor(Owner&) -> set_adaptor<owned_bits_t<Owner>, storage::borrowed>;

template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
sequence_adaptor(Bits&) -> sequence_adaptor<Bits, storage::borrowed>;

template<owner_reading<reading::sequence> Owner>
sequence_adaptor(Owner&) -> sequence_adaptor<owned_bits_t<Owner>, storage::borrowed>;

} // namespace xstd::detail::bits

namespace probe {

namespace real {

template<xstd::specialization_of_TN<xstd::detail::bits::contiguous_bit_container> Bits>
using set_view = xstd::detail::bits::set_adaptor<Bits, xstd::detail::bits::storage::borrowed>;

template<xstd::specialization_of_TN<xstd::detail::bits::contiguous_bit_container> Bits>
using span = xstd::detail::bits::sequence_adaptor<Bits, xstd::detail::bits::storage::borrowed>;

// The window argument pinned rather than defaulted, as the old bit_span pinned its third argument.
template<xstd::specialization_of_TN<xstd::detail::bits::contiguous_bit_container> Bits>
using pinned_span = xstd::detail::bits::sequence_adaptor<Bits, xstd::detail::bits::storage::borrowed, xstd::detail::bits::window::all>;

template<class Owner>
using bits_of = xstd::detail::bits::owned_bits_t<Owner>;

using words = xstd::borrowed_bits<std::uint64_t, 1>;

// R1. From borrowed words, through the requires-guarded guide.
static_assert(std::same_as<decltype(set_view(std::declval<words&>())), set_view<words>>);
static_assert(std::same_as<decltype(span(std::declval<words&>())), span<words>>);

// R2. From an owner, through the guide whose return type the owned_bits_t trait computes.
static_assert(std::same_as<decltype(set_view(std::declval<xstd::bit_set&>())), set_view<bits_of<xstd::bit_set>>>);
static_assert(std::same_as<decltype(span(std::declval<xstd::bit_vector&>())), span<bits_of<xstd::bit_vector>>>);

// R3. From a const owner, where the trait adds const to the storage.
static_assert(std::same_as<decltype(set_view(std::declval<xstd::bit_set const&>())), set_view<bits_of<xstd::bit_set const>>>);

// R4. The pinned window argument, from borrowed words and from an owner.
static_assert(std::same_as<decltype(pinned_span(std::declval<words&>())), pinned_span<words>>);
static_assert(std::same_as<decltype(pinned_span(std::declval<xstd::bit_vector&>())), pinned_span<bits_of<xstd::bit_vector>>>);

} // namespace real

// Stand-ins, each dropping one ingredient of the real shape, to find which one MSVC 17 refuses.
template<class T, std::size_t N>
struct box
{};

enum class storage : bool { owned,
                            borrowed,
};

template<class T>
struct owned_storage;

struct owner
{};

template<>
struct owned_storage<owner>
{
        using bits_type = box<int, 1>;
};

template<class Owner>
using owned_bits_t = std::conditional_t<std::is_const_v<Owner>, typename owned_storage<std::remove_const_t<Owner>>::bits_type const, typename owned_storage<std::remove_const_t<Owner>>::bits_type>;

template<class Owner>
concept is_owner = requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; };

template<class T>
concept is_box = xstd::specialization_of_TN<T, box>;

// S0: every ingredient of the real shape.
template<is_box Bits, storage S = storage::owned, class Derived = void>
class s0
{
public:
        explicit s0(Bits&);
        template<is_owner Owner>
        explicit s0(Owner&);
};

template<class Bits>
        requires (not is_owner<Bits>)
s0(Bits&) -> s0<Bits, storage::borrowed>;

template<is_owner Owner>
s0(Owner&) -> s0<owned_bits_t<Owner>, storage::borrowed>;

template<is_box Bits>
using v0 = s0<Bits, storage::borrowed>;

// S1: no requires-clause on the storage guide.
template<is_box Bits, storage S = storage::owned, class Derived = void>
class s1
{
public:
        explicit s1(Bits&);
        template<is_owner Owner>
        explicit s1(Owner&);
};

template<is_box Bits>
s1(Bits&) -> s1<Bits, storage::borrowed>;

template<is_owner Owner>
s1(Owner&) -> s1<owned_bits_t<Owner>, storage::borrowed>;

template<is_box Bits>
using v1 = s1<Bits, storage::borrowed>;

// S2: the owner guide names the member type directly instead of going through the conditional trait.
template<is_box Bits, storage S = storage::owned, class Derived = void>
class s2
{
public:
        explicit s2(Bits&);
        template<is_owner Owner>
        explicit s2(Owner&);
};

template<class Bits>
        requires (not is_owner<Bits>)
s2(Bits&) -> s2<Bits, storage::borrowed>;

template<is_owner Owner>
s2(Owner&) -> s2<typename owned_storage<Owner>::bits_type, storage::borrowed>;

template<is_box Bits>
using v2 = s2<Bits, storage::borrowed>;

// S3: the alias parameter unconstrained.
template<is_box Bits, storage S = storage::owned, class Derived = void>
class s3
{
public:
        explicit s3(Bits&);
        template<is_owner Owner>
        explicit s3(Owner&);
};

template<class Bits>
        requires (not is_owner<Bits>)
s3(Bits&) -> s3<Bits, storage::borrowed>;

template<is_owner Owner>
s3(Owner&) -> s3<owned_bits_t<Owner>, storage::borrowed>;

template<class Bits>
using v3 = s3<Bits, storage::borrowed>;

// S4: no defaulted Derived parameter after the pinned one.
template<is_box Bits, storage S = storage::owned>
class s4
{
public:
        explicit s4(Bits&);
        template<is_owner Owner>
        explicit s4(Owner&);
};

template<class Bits>
        requires (not is_owner<Bits>)
s4(Bits&) -> s4<Bits, storage::borrowed>;

template<is_owner Owner>
s4(Owner&) -> s4<owned_bits_t<Owner>, storage::borrowed>;

template<is_box Bits>
using v4 = s4<Bits, storage::borrowed>;

using b = box<int, 1>;

static_assert(std::same_as<decltype(v0(std::declval<b&>())), v0<b>>);
static_assert(std::same_as<decltype(v0(std::declval<owner&>())), v0<b>>);
static_assert(std::same_as<decltype(v1(std::declval<b&>())), v1<b>>);
static_assert(std::same_as<decltype(v1(std::declval<owner&>())), v1<b>>);
static_assert(std::same_as<decltype(v2(std::declval<b&>())), v2<b>>);
static_assert(std::same_as<decltype(v2(std::declval<owner&>())), v2<b>>);
static_assert(std::same_as<decltype(v3(std::declval<b&>())), v3<b>>);
static_assert(std::same_as<decltype(v3(std::declval<owner&>())), v3<b>>);
static_assert(std::same_as<decltype(v4(std::declval<b&>())), v4<b>>);
static_assert(std::same_as<decltype(v4(std::declval<owner&>())), v4<b>>);

} // namespace probe

BOOST_AUTO_TEST_SUITE(ViewAliasCtad)

BOOST_AUTO_TEST_CASE(EveryShapeCompiles)
{
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
