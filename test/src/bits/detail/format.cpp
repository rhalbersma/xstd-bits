//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>      // bit_array
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_set_view.hpp>   // bit_set_view
#include <xstd/bits/bit_span.hpp>       // bit_span
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <xstd/bits/bit_vector.hpp>     // bit_vector
#include <xstd/bits/dynamic_bitset.hpp> // dynamic_bitset
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <format>                       // format

BOOST_AUTO_TEST_SUITE(Format)

// Nothing here says anything about a container: the two proxies carry a formatter and [format.range.formatter] does the rest.

// [format.range.fmtkind] chooses range_format::set for a range with a key_type, so the set reading arrives at braces without being told, the way fmt's format_as does.
BOOST_AUTO_TEST_CASE(TheSetReadingFormatsInBraces)
{
        auto d = xstd::bit_set();
        d.insert(1UZ); d.insert(3UZ); d.insert(5UZ);
        BOOST_CHECK_EQUAL(std::format("{}", d), "{1, 3, 5}");

        auto s = xstd::bit_static_set<8>();
        s.insert(2UZ); s.insert(7UZ);
        BOOST_CHECK_EQUAL(std::format("{}", s), "{2, 7}");

        BOOST_CHECK_EQUAL(std::format("{}", xstd::bit_set()), "{}");
}

// And range_format::sequence otherwise, so the sequence reading arrives at brackets and prints every position, clear ones included, which is the whole difference between the two readings.
BOOST_AUTO_TEST_CASE(TheSequenceReadingFormatsInBrackets)
{
        auto v = xstd::bit_vector(4UZ);
        v[1] = true;
        BOOST_CHECK_EQUAL(std::format("{}", v), "[false, true, false, false]");

        auto a = xstd::bit_array<4>();
        a[2] = true;
        BOOST_CHECK_EQUAL(std::format("{}", a), "[false, false, true, false]");

        BOOST_CHECK_EQUAL(std::format("{}", xstd::bit_vector()), "[]");
}

// A view is a range over the same proxies, so it formats as its reading does and never as the owner's. The owner is a bitset here, committed to neither reading and so the one owner both views may refer into.
BOOST_AUTO_TEST_CASE(TheViewsFormatAsTheirReading)
{
        auto b = xstd::dynamic_bitset(4UZ);
        b.set(1UZ);
        b.set(3UZ);

        BOOST_CHECK_EQUAL(std::format("{}", xstd::bit_set_view(b)), "{1, 3}");
        BOOST_CHECK_EQUAL(std::format("{}", xstd::bit_span(b)),     "[false, true, false, true]");
}

// Deriving from formatter<size_t> and formatter<bool> rather than writing parse() is what keeps the spec, so the nested spec a range formatter forwards reaches the underlying one intact.
BOOST_AUTO_TEST_CASE(TheNestedSpecReachesTheUnderlyingFormatter)
{
        auto d = xstd::bit_set();
        d.insert(1UZ); d.insert(3UZ); d.insert(5UZ);
        BOOST_CHECK_EQUAL(std::format("{::#x}", d), "{0x1, 0x3, 0x5}");

        auto v = xstd::bit_vector(4UZ);
        v[1] = true;
        BOOST_CHECK_EQUAL(std::format("{::d}", v), "[0, 1, 0, 0]");
}

// A proxy formats on its own too, which is what the container's formatter is built out of.
BOOST_AUTO_TEST_CASE(AProxyFormatsAsItsValue)
{
        auto d = xstd::bit_set();
        d.insert(42UZ);
        BOOST_CHECK_EQUAL(std::format("{}",    *d.begin()), "42");
        BOOST_CHECK_EQUAL(std::format("{:>4}", *d.begin()), "  42");

        auto v = xstd::bit_vector(2UZ);
        v[1] = true;
        BOOST_CHECK_EQUAL(std::format("{}",    v[1]), "true");
        BOOST_CHECK_EQUAL(std::format("{:>7}", v[0]), "  false");
        BOOST_CHECK_EQUAL(std::format("{:d}",  v[1]), "1");
}

BOOST_AUTO_TEST_SUITE_END()
