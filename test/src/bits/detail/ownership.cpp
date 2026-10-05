//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/ownership.hpp> // owned_storage, owner, owner_of, owner_reading, owns, sequence_reading_tag, set_reading_tag, storage, view
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                       // derived_from, same_as

namespace {

// A storage no view here builds, which the owners below wrap.
struct fake_bits
{};

// A reading that refines the sequence one, as a further reading of the same bools would.
struct refined_reading_tag : xstd::bits::detail::sequence_reading_tag
{};

// Owners in name only, each committed to one reading.
struct set_owner
{};

struct sequence_owner
{};

struct refined_owner
{};

// A view in name only: it adapts a storage and owns none.
struct fake_view
{
        using adapted_type = fake_bits;
};

// An owner that also names what it adapts, as every owner built on an adaptor does.
struct adapting_owner
{
        using adapted_type = fake_bits;
};

} // namespace

template<>
struct xstd::bits::detail::owned_storage<set_owner>
{
        using bits_type = fake_bits;
        using reads     = set_reading_tag;
};

template<>
struct xstd::bits::detail::owned_storage<sequence_owner>
{
        using bits_type = fake_bits;
        using reads     = sequence_reading_tag;
};

template<>
struct xstd::bits::detail::owned_storage<refined_owner>
{
        using bits_type = fake_bits;
        using reads     = refined_reading_tag;
};

template<>
struct xstd::bits::detail::owned_storage<adapting_owner>
{
        using bits_type = fake_bits;
        using reads     = sequence_reading_tag;
};

BOOST_AUTO_TEST_SUITE(Ownership)

BOOST_AUTO_TEST_CASE(OwningIsOneOfTwoAnswers)
{
        static_assert(owns(xstd::bits::detail::storage::owned));
        static_assert(not owns(xstd::bits::detail::storage::borrowed));
        BOOST_CHECK(xstd::bits::detail::storage::owned != xstd::bits::detail::storage::borrowed);
}

// Neither reading refines the other, so the two stand apart.
BOOST_AUTO_TEST_CASE(TheReadingsAreTwoAndDistinct)
{
        using xstd::bits::detail::sequence_reading_tag;
        using xstd::bits::detail::set_reading_tag;
        static_assert(not std::same_as<set_reading_tag, sequence_reading_tag>);
        static_assert(not std::derived_from<set_reading_tag, sequence_reading_tag> and not std::derived_from<sequence_reading_tag, set_reading_tag>);
        BOOST_CHECK(true);
}

// A set owner and a sequence owner each refuse the other's view.
BOOST_AUTO_TEST_CASE(TheSetAndSequenceReadingsRejectEachOther)
{
        using xstd::bits::detail::owner_reading;
        using xstd::bits::detail::sequence_reading_tag;
        using xstd::bits::detail::set_reading_tag;
        static_assert(owner_reading<set_owner, set_reading_tag> and not owner_reading<set_owner, sequence_reading_tag>);
        static_assert(owner_reading<sequence_owner, sequence_reading_tag> and not owner_reading<sequence_owner, set_reading_tag>);
        BOOST_CHECK(true);
}

// A reading deriving from another answers wherever its base is asked for, and a base never answers for it.
BOOST_AUTO_TEST_CASE(ARefinedReadingIsAcceptedWhereItsBaseIsAskedFor)
{
        using xstd::bits::detail::owner_of;
        using xstd::bits::detail::owner_reading;
        using xstd::bits::detail::sequence_reading_tag;
        using xstd::bits::detail::set_reading_tag;
        static_assert(owner_reading<refined_owner, sequence_reading_tag> and owner_reading<refined_owner, refined_reading_tag>);
        static_assert(not owner_reading<refined_owner, set_reading_tag>);
        static_assert(not owner_reading<sequence_owner, refined_reading_tag>);
        static_assert(owner_of<refined_owner, fake_bits, sequence_reading_tag> and owner_of<refined_owner const, fake_bits const, sequence_reading_tag>);
        static_assert(not owner_of<refined_owner const, fake_bits, sequence_reading_tag>);
        BOOST_CHECK(true);
}

// An owner names its storage, const or not, and a view adapts one without owning it: no type is both.
BOOST_AUTO_TEST_CASE(AnOwnerIsNeverAView)
{
        using xstd::bits::detail::owner;
        using xstd::bits::detail::view;
        static_assert(owner<set_owner> and owner<sequence_owner const> and owner<adapting_owner> and not owner<fake_view>);
        static_assert(view<fake_view> and view<fake_view const> and not view<adapting_owner> and not view<set_owner>);
        static_assert(not owner<set_owner&> and not view<fake_view&> and not owner<fake_bits> and not view<fake_bits>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
