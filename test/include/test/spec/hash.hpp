//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_HASH_HPP
#define TEST_SPEC_HASH_HPP

#include <xstd/misc/ext/boost/hash2.hpp> // hasher
#include <boost/hash2/flavor.hpp>        // big_endian_flavor, default_flavor, little_endian_flavor
#include <boost/hash2/fnv1a.hpp>         // fnv1a_64
#include <boost/hash2/hash_append.hpp>   // hash_append
#include <boost/hash2/siphash.hpp>       // siphash_64
#include <boost/hash2/xxhash.hpp>        // xxhash_64
#include <boost/test/unit_test.hpp>      // BOOST_CHECK_EQUAL
#include <cstdint>                       // uint64_t

// A value hashed as its standard model is: the same message under every algorithm and flavor Hash2 offers.
namespace test::spec {

// What Hash2 makes of a value under an algorithm and a flavor.
template<class Hash, class Flavor, class X>
[[nodiscard]] constexpr auto flavored_digest(X const& x)
        -> Hash::result_type
{
        auto h = Hash();
        boost::hash2::hash_append(h, Flavor(), x);
        return h.result();
}

// A seed of the kind a container draws at random, fixed so that a failure reruns.
inline constexpr auto hash_seed = std::uint64_t{0x9E37'79B9'7F4A'7C15};

// Three algorithms, the native order and both fixed ones, and the public hasher seeded: the model's digest each time.
template<class X, class Model>
auto check_hashes_as(X const& x, Model const& model)
        -> void
{
        BOOST_CHECK_EQUAL((flavored_digest<boost::hash2::fnv1a_64, boost::hash2::default_flavor>(x)), (flavored_digest<boost::hash2::fnv1a_64, boost::hash2::default_flavor>(model)));
        BOOST_CHECK_EQUAL((flavored_digest<boost::hash2::xxhash_64, boost::hash2::little_endian_flavor>(x)), (flavored_digest<boost::hash2::xxhash_64, boost::hash2::little_endian_flavor>(model)));
        BOOST_CHECK_EQUAL((flavored_digest<boost::hash2::siphash_64, boost::hash2::big_endian_flavor>(x)), (flavored_digest<boost::hash2::siphash_64, boost::hash2::big_endian_flavor>(model)));
        auto const hasher = xstd::hasher<boost::hash2::siphash_64>(hash_seed);
        BOOST_CHECK_EQUAL(hasher(x), hasher(model));
}

} // namespace test::spec

#endif // TEST_SPEC_HASH_HPP
