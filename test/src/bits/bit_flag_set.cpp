//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/ext_int128.hpp>              // TEST_HAS_BOOST_INT128, uint128
#include <xstd/bits/bit/bit_convert.hpp>    // bit_convert
#include <xstd/bits/bit_flag_set.hpp>       // bit_flag_set
#include <xstd/bits/bit_flag_traits.hpp>    // bit_flag_traits
#include <xstd/bits/bit_key_traits.hpp>     // bit_key_traits
#include <xstd/bits/detail/mask_word.hpp>   // mask_word
#include <xstd/bits/detail/set_adaptor.hpp> // intersects
#include <xstd/bits/from_blocks.hpp>        // from_blocks
#include <xstd/filesystem.hpp>              // enum_traits, perm, perms
#include <xstd/ints/concepts/bit_mask.hpp>  // bit_mask
#include <boost/test/unit_test.hpp>         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                        // ranges::all_of, ranges::equal, ranges::includes, ranges::set_difference, ranges::set_intersection, ranges::set_symmetric_difference, ranges::set_union
#include <array>                            // array, to_array
#include <bit>                              // bit_cast, popcount
#include <bitset>                           // bitset
#include <compare>                          // is_gt, is_lt
#include <concepts>                         // convertible_to, same_as
#include <cstddef>                          // size_t
#include <cstdint>                          // uint16_t, uint8_t
#include <filesystem>                       // exists, path, perm_options, permissions, perms, remove, status, temp_directory_path
#include <format>                           // format
#include <fstream>                          // ofstream
#include <functional>                       // greater, less, ranges::greater
#include <iterator>                         // bidirectional_iterator, inserter, iter_reference_t, ranges::distance, ranges::next, ranges::prev
#include <random>                           // random_device
#include <ranges>                           // bidirectional_range, iota, ranges::swap, sized_range, views::reverse
#include <set>                              // set
#include <stdexcept>                        // out_of_range
#include <string_view>                      // string_view
#include <utility>                          // as_const, pair, to_underlying
#include <vector>                           // vector

namespace {

namespace fs  = std::filesystem;
namespace xfs = xstd::filesystem;

// A bitmask enumeration keying a flag type through its one-bit values, with no rank enumeration beside it.
enum class mode : std::uint8_t
{
        read  = 0x01,
        write = 0x02,
        exec  = 0x04,
};

class modes : public xstd::bit_flag_set<modes, mode, std::uint8_t, 3, xstd::bit_flag_traits<mode, 3>>
{
public:
        using bit_flag_set::bit_flag_set;
};

// Twelve bits wide, so a value coming in from the standard's type has no bit above set_uid.
class narrow_perms : public xstd::bit_flag_set<narrow_perms, xfs::perm, std::uint16_t, 12, xstd::bit_key_traits<xfs::perm>, fs::perms>
{
public:
        using bit_flag_set::bit_flag_set;
};

// The same sixteen positions converting with an unsigned integer and with a bitset rather than the enumeration.
class word_perms : public xstd::bit_flag_set<word_perms, xfs::perm, std::uint16_t, 16, xstd::bit_key_traits<xfs::perm>, std::uint16_t>
{
public:
        using bit_flag_set::bit_flag_set;
};

class bitset_perms : public xstd::bit_flag_set<bitset_perms, xfs::perm, std::uint16_t, 16, xstd::bit_key_traits<xfs::perm>, std::bitset<16>>
{
public:
        using bit_flag_set::bit_flag_set;
};

// Twelve bits wide against a sixteen-bit bitset, so a value coming in can have positions the flag type has not.
class narrow_bitset_perms : public xstd::bit_flag_set<narrow_bitset_perms, xfs::perm, std::uint16_t, 12, xstd::bit_key_traits<xfs::perm>, std::bitset<16>>
{
public:
        using bit_flag_set::bit_flag_set;
};

// The sixteen positions walked from the lowest flag up, under the set order rather than the enumeration's.
class ascending_perms : public xstd::bit_flag_set<ascending_perms, xfs::perm, std::uint16_t, 16, xstd::bit_key_traits<xfs::perm>, fs::perms, std::less<xfs::perm>> // NOLINT(modernize-use-transparent-functors): a flag type's comparator names its key
{
public:
        using bit_flag_set::bit_flag_set; // NOLINT(modernize-use-transparent-functors): the base's name carries the comparator
};

// Every name [fs.enum.perms] lists, beside the standard's own value of it.
constexpr auto names = std::to_array<std::pair<xfs::perms, fs::perms>>({
        {xfs::perms::none, fs::perms::none},
        {xfs::perms::owner_read, fs::perms::owner_read},
        {xfs::perms::owner_write, fs::perms::owner_write},
        {xfs::perms::owner_exec, fs::perms::owner_exec},
        {xfs::perms::owner_all, fs::perms::owner_all},
        {xfs::perms::group_read, fs::perms::group_read},
        {xfs::perms::group_write, fs::perms::group_write},
        {xfs::perms::group_exec, fs::perms::group_exec},
        {xfs::perms::group_all, fs::perms::group_all},
        {xfs::perms::others_read, fs::perms::others_read},
        {xfs::perms::others_write, fs::perms::others_write},
        {xfs::perms::others_exec, fs::perms::others_exec},
        {xfs::perms::others_all, fs::perms::others_all},
        {xfs::perms::all, fs::perms::all},
        {xfs::perms::set_uid, fs::perms::set_uid},
        {xfs::perms::set_gid, fs::perms::set_gid},
        {xfs::perms::sticky_bit, fs::perms::sticky_bit},
        {xfs::perms::mask, fs::perms::mask},
        {xfs::perms::unknown, fs::perms::unknown},
});

constexpr auto ranks = xstd::enum_traits<xfs::perm>::values;

// The model orders its keys as the flag type does, the highest flag first.
using model_type = std::set<xfs::perm, std::greater<>>;

// The flags a twelve-bit mask picks, as the flag type and as the model.
auto subset(std::size_t mask)
        -> std::pair<xfs::perms, model_type>
{
        auto nrv = std::pair<xfs::perms, model_type>();
        for (auto const k : ranks) {
                if (((mask >> std::to_underlying(k)) & 1UZ) != 0UZ) {
                        nrv.first |= xfs::perms(k);
                        nrv.second.insert(k);
                }
        }
        return nrv;
}

auto with(model_type m, xfs::perm k)
        -> model_type
{
        m.insert(k);
        return m;
}

auto without(model_type m, xfs::perm k)
        -> model_type
{
        m.erase(k);
        return m;
}

// The model's set algorithms, which the four operators are checked against.
template<class Algorithm>
auto combined(model_type const& a, model_type const& b, Algorithm algorithm)
        -> model_type
{
        auto nrv = model_type();
        algorithm(a, b, std::inserter(nrv, nrv.end()), std::ranges::greater());
        return nrv;
}

// One operator pair against the standard's type on either side, each answering as the standard's own operator.
auto meets_the_standards_type(xfs::perms a, xfs::perms b, fs::perms theirs)
        -> void
{
        auto const mine = fs::perms(a);
        BOOST_CHECK((a | theirs) == (mine | theirs));
        BOOST_CHECK((theirs | a) == (theirs | mine));
        BOOST_CHECK((a & theirs) == (mine & theirs));
        BOOST_CHECK((theirs & a) == (theirs & mine));
        BOOST_CHECK((a ^ theirs) == (mine ^ theirs));
        BOOST_CHECK((theirs ^ a) == (theirs ^ mine));
        BOOST_CHECK((a - theirs) == (mine & ~theirs));
        BOOST_CHECK((theirs - a) == (theirs & ~mine));
        BOOST_CHECK((a == theirs) == (mine == theirs));
        BOOST_CHECK((theirs == a) == (theirs == mine));
        BOOST_CHECK((a == theirs) == (a == b));
}

// One key of a subset, read and written through p[k] and probed by both contains, against the model.
auto agrees_at_key(xfs::perms const& p, model_type const& model, xfs::perm k)
        -> void
{
        BOOST_CHECK_EQUAL(p.contains(k), model.contains(k));
        BOOST_CHECK_EQUAL(p.contains(xfs::perms(k)), model.contains(k));
        BOOST_CHECK_EQUAL(p[k], model.contains(k));
        BOOST_CHECK_EQUAL(std::ranges::distance(p.begin(), p.lower_bound(k)), std::ranges::distance(model.begin(), model.lower_bound(k)));
        BOOST_CHECK_EQUAL(std::ranges::distance(p.begin(), p.upper_bound(k)), std::ranges::distance(model.begin(), model.upper_bound(k)));
        BOOST_CHECK_EQUAL((~p).contains(k), not model.contains(k));

        auto x = p;
        BOOST_CHECK((x[k] == true) == model.contains(k));
        BOOST_CHECK((x[k] == false) == not model.contains(k));
        x[k] = true;
        BOOST_CHECK(std::ranges::equal(x, with(model, k)));
        x[k] = false;
        BOOST_CHECK(std::ranges::equal(x, without(model, k)));
        x[k] = p[k];
        BOOST_CHECK(x == p);
}

// Two subsets, queried and combined by each operator and its compound form, against std::set's algorithms.
auto agrees_on_pair(std::size_t lhs, std::size_t rhs)
        -> void
{
        auto const [a, ma] = subset(lhs);
        auto const [b, mb] = subset(rhs);
        BOOST_CHECK_EQUAL(a.contains(b), std::ranges::includes(ma, mb, std::ranges::greater()));
        BOOST_CHECK_EQUAL(a.is_subset_of(b), std::ranges::includes(mb, ma, std::ranges::greater()));
        BOOST_CHECK_EQUAL(intersects(a, b), not combined(ma, mb, std::ranges::set_intersection).empty());
        BOOST_CHECK(std::ranges::equal(a | b, combined(ma, mb, std::ranges::set_union)));
        BOOST_CHECK(std::ranges::equal(a & b, combined(ma, mb, std::ranges::set_intersection)));
        BOOST_CHECK(std::ranges::equal(a ^ b, combined(ma, mb, std::ranges::set_symmetric_difference)));
        BOOST_CHECK(std::ranges::equal(a - b, combined(ma, mb, std::ranges::set_difference)));
        BOOST_CHECK_EQUAL(a == b, ma == mb);

        auto x = a;
        BOOST_CHECK(&(x |= b) == &x and x == (a | b));
        x = a;
        BOOST_CHECK(&(x &= b) == &x and x == (a & b));
        x = a;
        BOOST_CHECK(&(x ^= b) == &x and x == (a ^ b));
        x = a;
        BOOST_CHECK(&(x -= b) == &x and x == (a - b));
}

#ifdef __clang__

// Every standard name has a case; any other value, or an implementation's own enumerator, falls through to "".
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
#pragma clang diagnostic ignored "-Wswitch-default"

#endif

// The [fs.enum.perms] name of a value: a switch on the standard's type, or on ours through its conversion.
template<class Perms>
auto name_of(Perms p)
        -> std::string_view
{
        switch (p) {
                case Perms::none:
                        return "none";
                case Perms::owner_read:
                        return "owner_read";
                case Perms::owner_write:
                        return "owner_write";
                case Perms::owner_exec:
                        return "owner_exec";
                case Perms::owner_all:
                        return "owner_all";
                case Perms::group_read:
                        return "group_read";
                case Perms::group_write:
                        return "group_write";
                case Perms::group_exec:
                        return "group_exec";
                case Perms::group_all:
                        return "group_all";
                case Perms::others_read:
                        return "others_read";
                case Perms::others_write:
                        return "others_write";
                case Perms::others_exec:
                        return "others_exec";
                case Perms::others_all:
                        return "others_all";
                case Perms::all:
                        return "all";
                case Perms::set_uid:
                        return "set_uid";
                case Perms::set_gid:
                        return "set_gid";
                case Perms::sticky_bit:
                        return "sticky_bit";
                case Perms::mask:
                        return "mask";
                case Perms::unknown:
                        return "unknown";
        }
        return "";
}

#ifdef __clang__

#pragma clang diagnostic pop

#endif

// Code written against std::filesystem::perms, run on a fresh file with the type spelled Perms, and what it saw.
template<class Perms>
auto adjust_permissions(fs::path const& path)
        -> std::vector<fs::perms>
{
        std::ofstream(path).put('x');
        auto trace = std::vector<fs::perms>();

        Perms p = fs::status(path).permissions();
        trace.push_back(p);
        p &= ~(Perms::group_write | Perms::others_write);
        if ((p & Perms::owner_read) == Perms::none) {
                p |= Perms::owner_read;
        }
        trace.push_back(p);
        fs::permissions(path, p);
        trace.push_back(fs::status(path).permissions());
        fs::permissions(path, Perms::owner_exec, fs::perm_options::add);
        trace.push_back(fs::status(path).permissions());
        fs::permissions(path, Perms::owner_all | Perms::group_read, fs::perm_options::replace);
        trace.push_back(fs::status(path).permissions());
        fs::permissions(path, Perms::owner_write, fs::perm_options::remove);
        trace.push_back(fs::status(path).permissions());

        // Writable again, since Windows will not remove a read-only file.
        fs::permissions(path, Perms::owner_all, fs::perm_options::add);
        fs::remove(path);
        return trace;
}

} // namespace

BOOST_AUTO_TEST_SUITE(BitFlagSet)

// The standard's type and every flag type meet [bitmask.types]; the base cannot require it of an incomplete Derived.
BOOST_AUTO_TEST_CASE(BothTypesAreBitmaskTypes)
{
        static_assert(xstd::bit_mask<fs::perms>);
        static_assert(xstd::bit_mask<xfs::perms> and xstd::bit_mask<ascending_perms>);
        static_assert(xstd::bit_mask<modes>);
        static_assert(xstd::bit_mask<narrow_perms> and xstd::bit_mask<narrow_bitset_perms>);
        static_assert(xstd::bit_mask<word_perms> and xstd::bit_mask<bitset_perms>);
        static_assert(xstd::bit_convert<std::uint16_t>(xfs::perms{}) == 0U and xstd::bit_convert<std::uint16_t>(xfs::perms::none) == 0U);

        BOOST_CHECK(true);
}

// Each name has the standard's value, the mask and unknown included, in a constant expression and at run time.
BOOST_AUTO_TEST_CASE(EveryNameHasTheStandardsValue)
{
        static_assert(std::ranges::all_of(names, [](auto const& name) noexcept -> bool { return name.first == name.second; }));
        static_assert(xstd::bit_convert<std::uint16_t>(xfs::perms::unknown) == 0xFFFFU);
        static_assert(xstd::bit_convert<std::uint16_t>(xfs::perms::mask) == 07777U);
        for (auto const& [ours, theirs] : names) {
                BOOST_CHECK(ours == theirs);
                BOOST_CHECK(fs::perms(ours) == theirs);
                BOOST_CHECK(xfs::perms(theirs) == ours);
                BOOST_CHECK_EQUAL(name_of(ours), name_of(theirs));
        }
        BOOST_CHECK_EQUAL(name_of(xfs::perms::owner_read | xfs::perms::group_read), "");
}

// Every 16-bit value converts in from the standard's type and back unchanged, unnamed bits included.
BOOST_AUTO_TEST_CASE(EverySixteenBitValueRoundTrips)
{
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0U, 0x10000U)) {
                auto const theirs     = static_cast<fs::perms>(word);
                xfs::perms const ours = theirs;
                if (fs::perms(ours) != theirs or xstd::bit_convert<std::uint16_t>(ours) != word or ours != xfs::perms(xstd::from_blocks, static_cast<std::uint16_t>(word))) {
                        ++mismatches;
                }
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);
}

// Each operator takes the standard's type on either side, and answers as the standard's own operator does.
BOOST_AUTO_TEST_CASE(EachOperatorMeetsTheStandardsTypeInBothOrders)
{
        static_assert(std::same_as<decltype(xfs::perms::owner_read | fs::perms::owner_write), xfs::perms>);
        static_assert(std::same_as<decltype(fs::perms::owner_write | xfs::perms::owner_read), xfs::perms>);
        static_assert(std::same_as<decltype(xfs::perms::owner_read & fs::perms::owner_write), xfs::perms>);
        static_assert(std::same_as<decltype(fs::perms::owner_write & xfs::perms::owner_read), xfs::perms>);
        static_assert(std::same_as<decltype(xfs::perms::owner_read ^ fs::perms::owner_write), xfs::perms>);
        static_assert(std::same_as<decltype(fs::perms::owner_write ^ xfs::perms::owner_read), xfs::perms>);
        static_assert(std::same_as<decltype(xfs::perms::owner_read - fs::perms::owner_write), xfs::perms>);
        static_assert(std::same_as<decltype(fs::perms::owner_write - xfs::perms::owner_read), xfs::perms>);
        static_assert(std::same_as<decltype(xfs::perms::owner_read == fs::perms::owner_write), bool>);
        static_assert(std::same_as<decltype(fs::perms::owner_write == xfs::perms::owner_read), bool>);
        static_assert(std::same_as<decltype(fs::perms::owner_write != xfs::perms::owner_read), bool>);

        for (auto const& [a, ours] : names) {
                for (auto const& [b, theirs] : names) {
                        meets_the_standards_type(a, b, theirs);
                }
        }
}

// & and - read the standard's value below sixteen bits, where it can meet a flag, and == finds higher bits unequal.
BOOST_AUTO_TEST_CASE(AndMinusAndEqualityTakeAValueWithHigherBits)
{
        auto const p = xfs::perms::owner_all | xfs::perms::group_write;
        BOOST_CHECK((p & ~fs::perms::group_write) == xfs::perms::owner_all);
        BOOST_CHECK((~fs::perms::group_write & p) == xfs::perms::owner_all);
        BOOST_CHECK((p - ~fs::perms::none) == xfs::perms::none);
        BOOST_CHECK((p - ~fs::perms::owner_all) == xfs::perms::owner_all);
        BOOST_CHECK((xfs::perms::unknown & ~fs::perms::none) == xfs::perms::unknown);
        BOOST_CHECK(not(xfs::perms::unknown == ~fs::perms::none));
        BOOST_CHECK(not(~fs::perms::none == xfs::perms::unknown));
        BOOST_CHECK(xfs::perms::unknown != ~fs::perms::none);
        BOOST_CHECK(xfs::perms::unknown == fs::perms::unknown);

        // The compound forms, against the standard's own on the same values.
        auto x      = p;
        auto theirs = fs::perms(p);
        x &= ~fs::perms::group_write;
        theirs &= ~fs::perms::group_write;
        BOOST_CHECK(x == theirs);
        BOOST_CHECK(&(x -= ~fs::perms::owner_all) == &x);
        theirs &= fs::perms::owner_all;
        BOOST_CHECK(x == theirs);
        BOOST_CHECK(x == xfs::perms::owner_all);
        static_assert(std::same_as<decltype(x &= ~fs::perms::none), xfs::perms&>);
        static_assert(std::same_as<decltype(x -= ~fs::perms::none), xfs::perms&>);
}

// The same code, once against the standard's type and once respelled, leaves a real file the same way.
BOOST_AUTO_TEST_CASE(RespelledCodeBehavesAsTheStandardsTypeOnARealFile)
{
        auto const path       = fs::temp_directory_path() / std::format("xstd-bits-bit-flag-set-{}.tmp", std::random_device()());
        auto const std_trace  = adjust_permissions<fs::perms>(path);
        auto const xstd_trace = adjust_permissions<xfs::perms>(path);
        BOOST_CHECK(std_trace == xstd_trace);
        BOOST_CHECK(not fs::exists(path));
}

// Every subset of the twelve flags, walked, counted, probed, complemented and written through p[k] against std::set.
BOOST_AUTO_TEST_CASE(EverySubsetAgreesWithStdSet)
{
        constexpr auto beyond = std::bit_cast<xfs::perm>(std::uint8_t{12});
        for (auto const mask : std::views::iota(0UZ, 1UZ << ranks.size())) {
                auto const [p, model] = subset(mask);
                BOOST_CHECK(std::ranges::equal(p, model));
                BOOST_CHECK(std::ranges::equal(std::views::reverse(p), std::views::reverse(model)));
                BOOST_CHECK_EQUAL(p.size(), model.size());
                BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(p), mask);
                BOOST_CHECK(not p.contains(beyond));
                BOOST_CHECK(not p[beyond]);

                // The complement stays within the sixteen bits, and its named flags are the rest of the twelve.
                auto const complement = ~p;
                BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(complement), static_cast<std::uint16_t>(~mask));
                BOOST_CHECK_EQUAL(complement.size(), ranks.size() - model.size());
                for (auto const k : ranks) {
                        agrees_at_key(p, model, k);
                }
        }
}

// The queries and the four operators over pairs of subsets, against std::set's algorithms.
BOOST_AUTO_TEST_CASE(PairsOfSubsetsAgreeWithStdSet)
{
        for (auto lhs = 0UZ; lhs < (1UZ << ranks.size()); lhs += 31UZ) {
                for (auto rhs = 0UZ; rhs < (1UZ << ranks.size()); rhs += 37UZ) {
                        agrees_on_pair(lhs, rhs);
                }
        }
}

// With a single flag, contains asks the same question whether it is given the key or the constant.
BOOST_AUTO_TEST_CASE(BothContainsAgreeOnASingleFlag)
{
        auto const p = xfs::perms::owner_read | xfs::perms::group_write;
        static_assert((xfs::perms::owner_read | xfs::perms::group_write).contains(xfs::perm::owner_read));
        BOOST_CHECK(p.contains(xfs::perm::owner_read) and p.contains(xfs::perms::owner_read));
        BOOST_CHECK(not p.contains(xfs::perm::owner_write) and not p.contains(xfs::perms::owner_write));
        BOOST_CHECK(not p.contains(xfs::perms::owner_all));
        BOOST_CHECK(intersects(p, xfs::perms::owner_all));
        BOOST_CHECK(p.contains(fs::perms::group_write));
        BOOST_CHECK(xfs::perms::owner_read.is_subset_of(p));
}

// The unnamed bits are in the word and survive every operator, but iteration and size see the named flags alone.
BOOST_AUTO_TEST_CASE(UnnamedBitsAreKeptButNotIterated)
{
        auto const p = xfs::perms::unknown;
        BOOST_CHECK_EQUAL(p.size(), 12UZ);
        BOOST_CHECK(std::ranges::equal(p, std::views::reverse(ranks)));
        BOOST_CHECK(p != xfs::perms::mask);
        BOOST_CHECK(~p == xfs::perms::none);
        BOOST_CHECK(~xfs::perms::none == xfs::perms::unknown);
        BOOST_CHECK((p - xfs::perms::mask).empty());
        BOOST_CHECK((p - xfs::perms::mask) != xfs::perms::none);
        BOOST_CHECK(std::ranges::equal(p - xfs::perms::mask, std::array<xfs::perm, 0>()));
}

// The value sees all sixteen bits: ==, <=>, ~ and the conversions; the range and its lookups see the twelve keys.
BOOST_AUTO_TEST_CASE(TheValueSeesUnnamedPositionsAndTheRangeDoesNot)
{
        constexpr auto beyond = std::bit_cast<xfs::perm>(std::uint8_t{12});
        auto const high       = xfs::perms(xstd::from_blocks, std::uint16_t{0xF000});
        static_assert(xfs::perms::max_size() == 12UZ);
        BOOST_CHECK(high.empty() and high.begin() == high.end() and (high | xfs::perms::owner_read).size() == 1UZ);
        BOOST_CHECK(high != xfs::perms::none);
        BOOST_CHECK(std::is_gt(high <=> xfs::perms::none));
        BOOST_CHECK(std::is_lt(xfs::perms::mask <=> xfs::perms::unknown));
        BOOST_CHECK(~high == xfs::perms::mask);
        BOOST_CHECK((high | xfs::perms::mask) == xfs::perms::unknown);
        BOOST_CHECK(fs::perms(high) == (fs::perms::unknown & ~fs::perms::mask));
        BOOST_CHECK(not xfs::perms::unknown.contains(beyond));
        BOOST_CHECK(xfs::perms::unknown.find(beyond) == xfs::perms::unknown.end()); // NOLINT(readability-container-contains): find stopping at the named keys is the check
        BOOST_CHECK(*xfs::perms::unknown.begin() == xfs::perm::set_uid and *xfs::perms::unknown.rbegin() == xfs::perm::others_exec);
        BOOST_CHECK(xfs::perms::unknown.lower_bound(beyond) == xfs::perms::unknown.begin());

        // Walking down, each scan that finds no named flag below where it starts ends at end(), not at the width.
        auto const one = high | xfs::perms::owner_read;
        BOOST_CHECK(std::ranges::equal(one, std::array{xfs::perm::owner_read}));
        BOOST_CHECK(std::ranges::next(one.begin()) == one.end() and *std::ranges::prev(one.end()) == xfs::perm::owner_read);
        BOOST_CHECK(one.upper_bound(xfs::perm::owner_read) == one.end() and one.lower_bound(xfs::perm::owner_exec) == one.end());
        BOOST_CHECK(high.lower_bound(beyond) == high.end() and high.upper_bound(beyond) == high.end());

        // perms::unknown leaves through the standard's type and comes back unchanged, its four unnamed bits included.
        xfs::perms const back = fs::perms(xfs::perms::unknown);
        BOOST_CHECK(back == xfs::perms::unknown);
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(back), 0xFFFFU);
}

// insert refuses a position no key names, as a set refuses a key past its max_size(), and the word is untouched.
BOOST_AUTO_TEST_CASE(InsertingAnUnnamedPositionThrows)
{
        constexpr auto beyond = std::bit_cast<xfs::perm>(std::uint8_t{12});
        auto p                = xfs::perms::owner_read;
        BOOST_CHECK_THROW(p.insert(beyond), std::out_of_range);
        BOOST_CHECK(p == xfs::perms::owner_read);
}

// The set reading's members come with the base: insert, erase, find, the reverse range, clear and swap.
BOOST_AUTO_TEST_CASE(TheInheritedSetInterfaceWorks)
{
        auto p = xfs::perms(xstd::from_blocks, std::uint16_t{0xF000});
        BOOST_CHECK(p.insert(xfs::perm::owner_read).second);
        BOOST_CHECK(not p.insert(xfs::perm::owner_read).second);
        p.insert(xfs::perm::group_exec);
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(p), 0xF108U);
        BOOST_CHECK(*p.find(xfs::perm::group_exec) == xfs::perm::group_exec);
        BOOST_CHECK(p.find(xfs::perm::others_read) == p.end()); // NOLINT(readability-container-contains): find is the inherited member under test
        BOOST_CHECK(*p.rbegin() == xfs::perm::group_exec);
        BOOST_CHECK(std::ranges::equal(std::views::reverse(p), std::array{xfs::perm::group_exec, xfs::perm::owner_read}));
        BOOST_CHECK_EQUAL(p.erase(xfs::perm::owner_read), 1UZ);
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(p), 0xF008U);

        auto q = xfs::perms::owner_all;
        swap(p, q);
        BOOST_CHECK(p == xfs::perms::owner_all and xstd::bit_convert<std::uint16_t>(q) == 0xF008U);
        std::ranges::swap(p, q);
        BOOST_CHECK(q == xfs::perms::owner_all);
        p.clear();
        BOOST_CHECK(p == xfs::perms::none);
}

// A key or the interop mask on either side picks one operator, and each returns the flag type.
BOOST_AUTO_TEST_CASE(MixedOperatorsAreUnambiguous)
{
        constexpr auto k = xfs::perm::owner_read;
        auto const p     = xfs::perms::group_all;
        static_assert(std::same_as<decltype(p | p), xfs::perms> and std::same_as<decltype(~p), xfs::perms>);
        static_assert(std::same_as<decltype(p | k), xfs::perms> and std::same_as<decltype(k | p), xfs::perms>);
        static_assert(std::same_as<decltype(p & k), xfs::perms> and std::same_as<decltype(k & p), xfs::perms>);
        static_assert(std::same_as<decltype(p ^ k), xfs::perms> and std::same_as<decltype(k ^ p), xfs::perms>);
        static_assert(std::same_as<decltype(p - k), xfs::perms> and std::same_as<decltype(k - p), xfs::perms>);
        static_assert(std::same_as<decltype(word_perms() | k), word_perms> and std::same_as<decltype(k - word_perms()), word_perms>);
        BOOST_CHECK((p | k) == (p | xfs::perms::owner_read) and (k | p) == (p | xfs::perms::owner_read));
        BOOST_CHECK((p & k) == xfs::perms::none and (k & (p | k)) == xfs::perms::owner_read);
        BOOST_CHECK((p ^ k) == (k ^ p) and (p - k) == p and (k - p) == xfs::perms::owner_read);
        BOOST_CHECK((word_perms() | k) == std::uint16_t{0x100});

        // The compound forms take the standard's type as the binary ones do, | and ^ with no bit at or above N.
        auto x = p;
        BOOST_CHECK(&(x |= fs::perms::owner_read) == &x and x == (p | xfs::perms::owner_read));
        BOOST_CHECK(&(x ^= fs::perms::group_all) == &x and x == xfs::perms::owner_read);
        BOOST_CHECK(&(x |= p) == &x and &(x ^= k) == &x and x == p);
        static_assert(std::same_as<decltype(x |= fs::perms::none), xfs::perms&>);
        static_assert(std::same_as<decltype(x ^= fs::perms::none), xfs::perms&>);
}

// p[k] is a proxy nested in the flag type, a bool by its one conversion, and compares through the built-in ==.
BOOST_AUTO_TEST_CASE(TheSubscriptIsANestedProxyForABool)
{
        using base_type = xstd::bit_flag_set<xfs::perms, xfs::perm, std::uint16_t, 16, xstd::bit_key_traits<xfs::perm>, fs::perms>;
        auto p          = xfs::perms::owner_read;
        static_assert(std::same_as<decltype(p[xfs::perm::owner_read]), xfs::perms::reference>);
        static_assert(std::same_as<xfs::perms::reference, base_type::reference>);
        static_assert(std::same_as<xfs::perms::reference::value_type, bool>);
        static_assert(std::convertible_to<xfs::perms::reference, bool>);
        static_assert(std::same_as<decltype(p[xfs::perm::owner_read] == true), bool>);
        static_assert(std::same_as<decltype(std::as_const(p)[xfs::perm::owner_read]), bool>);

        BOOST_CHECK(p[xfs::perm::owner_read] == true);
        BOOST_CHECK(p[xfs::perm::owner_write] == false);
        BOOST_CHECK(p[xfs::perm::owner_read] != p[xfs::perm::owner_write]);
        p[xfs::perm::owner_write] = p[xfs::perm::owner_read];
        BOOST_CHECK(p == (xfs::perms::owner_read | xfs::perms::owner_write));
        if (p[xfs::perm::owner_exec]) {
                BOOST_CHECK(false);
        }
}

// The flag type is a bidirectional range of its rank enumeration, whose read-only proxy converts to each key.
BOOST_AUTO_TEST_CASE(TheFlagTypeIsABidirectionalRangeOfItsKeys)
{
        static_assert(std::bidirectional_iterator<xfs::perms::iterator>);
        static_assert(std::ranges::bidirectional_range<xfs::perms const>);
        static_assert(std::ranges::sized_range<xfs::perms const>);
        static_assert(std::same_as<std::iter_reference_t<xfs::perms::iterator>, xfs::perms::const_reference>);
        static_assert(std::convertible_to<xfs::perms::const_reference, xfs::perm>);

        auto const p = xfs::perms::owner_all;
        auto it      = p.begin();
        BOOST_CHECK(*it++ == xfs::perm::owner_read);
        BOOST_CHECK(*it == xfs::perm::owner_write);
        BOOST_CHECK(++it != p.end());
        BOOST_CHECK(++it == p.end());
}

// The set prints as its keys do, through the rank enumeration's own formatter, the highest flag first.
BOOST_AUTO_TEST_CASE(TheFlagTypeFormatsItsFlagsByName)
{
        BOOST_CHECK_EQUAL(std::format("{}", xfs::perms::owner_read | xfs::perms::owner_write), "{owner_read, owner_write}");
        BOOST_CHECK_EQUAL(std::format("{}", xfs::perms::none), "{}");
        BOOST_CHECK_EQUAL(std::format("{}", xfs::perms::set_uid | xfs::perms::others_exec), "{set_uid, others_exec}");
}

// The walk runs from the highest flag down, as ls -l reads the mode: owner_read first and others_exec last.
BOOST_AUTO_TEST_CASE(TheFlagTypeIteratesFromTheHighestFlagDown)
{
        static_assert(std::same_as<xfs::perms::key_compare, std::greater<xfs::perm>>);
        constexpr auto rwx = std::to_array({xfs::perm::owner_read, xfs::perm::owner_write, xfs::perm::owner_exec, xfs::perm::group_read, xfs::perm::group_write, xfs::perm::group_exec, xfs::perm::others_read, xfs::perm::others_write, xfs::perm::others_exec});
        BOOST_CHECK(std::ranges::equal(xfs::perms::all, rwx));
        BOOST_CHECK(xfs::perms::mask.front() == xfs::perm::set_uid and xfs::perms::mask.back() == xfs::perm::others_exec);
        BOOST_CHECK_EQUAL(std::format("{}", xfs::perms::all), "{owner_read, owner_write, owner_exec, group_read, group_write, group_exec, others_read, others_write, others_exec}");
}

// <=> on two flag values is the enumeration's on their underlying words, and the mixed < through the conversion agrees.
BOOST_AUTO_TEST_CASE(ThreeWayComparisonIsTheEnumerations)
{
        constexpr auto others = std::to_array<std::uint16_t>({0x0000, 0x0001, 0x0007, 0x0100, 0x01FF, 0x0800, 0x0FFF, 0x1000, 0xF000, 0xFFFF});
        auto mismatches       = 0UZ;
        for (auto const word : std::views::iota(0U, 0x10000U)) {
                auto const p = xfs::perms(xstd::from_blocks, static_cast<std::uint16_t>(word));
                for (auto const other : others) {
                        auto const q        = xfs::perms(xstd::from_blocks, other);
                        auto const expected = std::to_underlying(fs::perms(p)) <=> std::to_underlying(fs::perms(q));
                        auto const mirrored = std::to_underlying(fs::perms(q)) <=> std::to_underlying(fs::perms(p));
                        if ((p <=> q) != expected or (q <=> p) != mirrored or (p < fs::perms(q)) != std::is_lt(expected)) {
                                ++mismatches;
                        }
                }
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);
}

// With std::less, the walk runs from the lowest flag up and <=> is the lexicographic set order over it.
BOOST_AUTO_TEST_CASE(AnAscendingFlagTypeIteratesUpAndOrdersAsASet)
{
        static_assert(std::same_as<ascending_perms::key_compare, std::less<xfs::perm>>); // NOLINT(modernize-use-transparent-functors): a flag type's comparator names its key
        ascending_perms const p = fs::perms::owner_all;
        BOOST_CHECK(std::ranges::equal(p, std::array{xfs::perm::owner_exec, xfs::perm::owner_write, xfs::perm::owner_read}));
        BOOST_CHECK_EQUAL(std::format("{}", p), "{owner_exec, owner_write, owner_read}");
        BOOST_CHECK(std::ranges::equal(ascending_perms(xstd::from_blocks, std::uint16_t{0xF001}), std::array{xfs::perm::others_exec}));

        // {others_exec, set_uid} precedes {others_write} as a set, though its block is the larger one.
        auto const lower = fs::perms::others_exec | fs::perms::set_uid;
        auto const upper = fs::perms::others_write;
        BOOST_CHECK(std::is_lt(ascending_perms(lower) <=> ascending_perms(upper)));
        BOOST_CHECK(std::is_gt(xfs::perms(lower) <=> xfs::perms(upper)));
        BOOST_CHECK(lower > upper);
}

// A width of twelve takes every value whose bits are all below it, and its complement stays within it.
BOOST_AUTO_TEST_CASE(ANarrowerWidthTakesEveryValueBelowIt)
{
        static_assert(sizeof(narrow_perms) == sizeof(std::uint16_t));
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0U, 0x1000U)) {
                auto const theirs       = static_cast<fs::perms>(word);
                narrow_perms const ours = theirs;
                if (fs::perms(ours) != theirs or ours.size() != static_cast<std::size_t>(std::popcount(word))) {
                        ++mismatches;
                }
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);
        BOOST_CHECK(~narrow_perms() == fs::perms::mask);
        BOOST_CHECK(fs::perms::mask == ~narrow_perms());

        // Bits above twelve make a value unequal and are dropped by &, where nothing of the flag set meets them.
        BOOST_CHECK(~narrow_perms() != fs::perms::none);
        BOOST_CHECK(~narrow_perms() != fs::perms::unknown);
        BOOST_CHECK((~narrow_perms() & fs::perms::unknown) == fs::perms::mask);
}

// A mask enumeration keys a flag type directly: iteration yields one-bit values, and the type has no interop.
BOOST_AUTO_TEST_CASE(AMaskEnumerationKeysAFlagTypeDirectly)
{
        static_assert(not std::convertible_to<modes, mode>);
        static_assert(not std::convertible_to<mode, modes>);
        auto const m = modes(mode::read) | modes(mode::exec);
        BOOST_CHECK(std::ranges::equal(m, std::array{mode::exec, mode::read}));
        BOOST_CHECK(m.contains(mode::exec));
        BOOST_CHECK(not m.contains(mode::write));
        BOOST_CHECK(not m.contains(std::bit_cast<mode>(std::uint8_t{0x08})));
        BOOST_CHECK(~m == modes(mode::write));
        BOOST_CHECK(~modes() == modes(xstd::from_blocks, std::uint8_t{0x07}));
}

#ifdef TEST_HAS_BOOST_INT128

namespace {

// A class-type block, whose namespace declares operator==(uint128, bool) and its mirror.
class lamps : public xstd::bit_flag_set<lamps, mode, boost::int128::uint128, 128, xstd::bit_flag_traits<mode, 3>>
{
public:
        using bit_flag_set::bit_flag_set;
};

} // namespace

// p[k] == true is the built-in comparison: the nested proxy brings no block's namespace into ADL to tie with it.
BOOST_AUTO_TEST_CASE(TheSubscriptComparesWithBoolOverAClassTypeBlock)
{
        static_assert(xstd::bit_mask<lamps>);
        auto l         = lamps();
        l[mode::write] = true;
        BOOST_CHECK(l[mode::write] == true);
        BOOST_CHECK(l[mode::read] == false);
        BOOST_CHECK(true == l[mode::write]);
        l[mode::write] = false;
        BOOST_CHECK(l[mode::write] == false);
}

#endif

// The interop is any of [bitmask.types]'s three forms: an enumeration, an unsigned integer or a bitset, and no other.
BOOST_AUTO_TEST_CASE(TheInteropIsAnEnumerationAnUnsignedIntegerOrABitset)
{
        static_assert(xstd::bits::detail::mask_word<fs::perms>);
        static_assert(xstd::bits::detail::mask_word<std::uint16_t>);
        static_assert(xstd::bits::detail::mask_word<std::bitset<16>>);
        static_assert(not xstd::bits::detail::mask_word<int>);
        static_assert(not xstd::bits::detail::mask_word<modes>);
        static_assert(std::same_as<word_perms::interop_type, std::uint16_t>);
        static_assert(std::same_as<bitset_perms::interop_type, std::bitset<16>>);
        BOOST_CHECK(true);
}

// Every 16-bit value converts in from an unsigned integer and from a bitset, and back unchanged.
BOOST_AUTO_TEST_CASE(EverySixteenBitValueRoundTripsThroughAWordAndABitset)
{
        static_assert(static_cast<std::uint16_t>(word_perms(std::uint16_t{0x0123})) == 0x0123);
        static_assert(word_perms(std::uint16_t{0x0123}) == std::uint16_t{0x0123});
        static_assert(std::bitset<16>(bitset_perms(std::bitset<16>(0x0123))) == std::bitset<16>(0x0123));
        static_assert(bitset_perms(std::bitset<16>(0x0123)) == std::bitset<16>(0x0123));
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0U, 0x10000U)) {
                auto const value          = static_cast<std::uint16_t>(word);
                auto const bits           = std::bitset<16>(word);
                word_perms const from_w   = value;
                bitset_perms const from_b = bits;
                if (static_cast<std::uint16_t>(from_w) != value or xstd::bit_convert<std::uint16_t>(from_w) != value or std::bitset<16>(from_b) != bits or xstd::bit_convert<std::uint16_t>(from_b) != value or from_b != bits) {
                        ++mismatches;
                }
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);
}

// Each operator takes the word or the bitset on either side, and answers as the enumeration does.
BOOST_AUTO_TEST_CASE(EachOperatorMeetsAWordAndABitsetInBothOrders)
{
        auto const p = word_perms(xstd::from_blocks, std::uint16_t{0x0F0});
        auto const q = bitset_perms(xstd::from_blocks, std::uint16_t{0x0F0});
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(p | std::uint16_t{0x00F}) == 0x0FF and xstd::bit_convert<std::uint16_t>(std::uint16_t{0x00F} | p) == 0x0FF);
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(q | std::bitset<16>(0x00F)) == 0x0FF and xstd::bit_convert<std::uint16_t>(std::bitset<16>(0x00F) | q) == 0x0FF);
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(p & std::uint16_t{0x030}) == 0x030 and xstd::bit_convert<std::uint16_t>(std::bitset<16>(0x030) & q) == 0x030);
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(p ^ std::uint16_t{0x0FF}) == 0x00F and xstd::bit_convert<std::uint16_t>(std::bitset<16>(0x0FF) ^ q) == 0x00F);
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(p - std::uint16_t{0x030}) == 0x0C0 and xstd::bit_convert<std::uint16_t>(q - std::bitset<16>(0x030)) == 0x0C0);
}

// Against a sixteen-bit bitset, a twelve-bit flag type reads the low twelve, and finds any higher position unequal.
BOOST_AUTO_TEST_CASE(ANarrowerWidthReadsTheLowPositionsOfABitset)
{
        auto const high = std::bitset<16>(0x1001);
        auto const p    = narrow_bitset_perms(xstd::from_blocks, std::uint16_t{0x0FFF});
        BOOST_CHECK(std::bitset<16>(p & high) == std::bitset<16>(0x001));
        BOOST_CHECK(std::bitset<16>(p - high) == std::bitset<16>(0xFFE));
        BOOST_CHECK(not(narrow_bitset_perms(xstd::from_blocks, std::uint16_t{0x001}) == high));
        BOOST_CHECK(narrow_bitset_perms(xstd::from_blocks, std::uint16_t{0x001}) == std::bitset<16>(0x001));
        BOOST_CHECK(std::bitset<16>(~narrow_bitset_perms()) == std::bitset<16>(0x0FFF));
}

BOOST_AUTO_TEST_SUITE_END()
