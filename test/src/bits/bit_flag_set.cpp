//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/ext_int128.hpp>           // TEST_HAS_BOOST_INT128, uint128
#include <xstd/bits/bit_flag_set.hpp>    // bit_flag_set
#include <xstd/bits/bit_flag_traits.hpp> // bit_flag_traits
#include <xstd/bits/bit_key_traits.hpp>  // bit_key_traits
#include <xstd/filesystem.hpp>           // enum_traits, perm, perms
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                     // ranges::all_of, ranges::equal, ranges::includes, ranges::set_difference, ranges::set_intersection, ranges::set_symmetric_difference, ranges::set_union
#include <array>                         // array, to_array
#include <bit>                           // bit_cast, popcount
#include <concepts>                      // convertible_to, same_as
#include <cstddef>                       // size_t
#include <cstdint>                       // uint16_t, uint8_t
#include <filesystem>                    // exists, path, perm_options, permissions, perms, remove, status, temp_directory_path
#include <format>                        // format
#include <fstream>                       // ofstream
#include <iterator>                      // forward_iterator, inserter, iter_reference_t
#include <random>                        // random_device
#include <ranges>                        // forward_range, iota, sized_range
#include <set>                           // set
#include <string_view>                   // string_view
#include <utility>                       // as_const, pair, to_underlying
#include <vector>                        // vector

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

// [bitmask.types]: the three operators, the complement and their compound forms giving back the type, and a zero value.
template<class T>
concept bitmask_type = requires (T x, T y) {
        { x | y } -> std::same_as<T>;
        { x & y } -> std::same_as<T>;
        { x ^ y } -> std::same_as<T>;
        { ~x } -> std::same_as<T>;
        { x |= y } -> std::same_as<T&>;
        { x &= y } -> std::same_as<T&>;
        { x ^= y } -> std::same_as<T&>;
        { T{} == x } -> std::convertible_to<bool>;
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

using model_type = std::set<xfs::perm>;

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
        algorithm(a, b, std::inserter(nrv, nrv.end()));
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
        BOOST_CHECK_EQUAL(a.contains(b), std::ranges::includes(ma, mb));
        BOOST_CHECK_EQUAL(a.is_subset_of(b), std::ranges::includes(mb, ma));
        BOOST_CHECK_EQUAL(a.intersects(b), not combined(ma, mb, std::ranges::set_intersection).empty());
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

// The standard's type meets [bitmask.types], and so does the flag type spelled the same way.
BOOST_AUTO_TEST_CASE(BothTypesAreBitmaskTypes)
{
        static_assert(bitmask_type<fs::perms>);
        static_assert(bitmask_type<xfs::perms>);
        static_assert(bitmask_type<modes>);
        static_assert(xfs::perms{} == xfs::perms::none);

        BOOST_CHECK(true);
}

// Each name has the standard's value, the mask and unknown included, in a constant expression and at run time.
BOOST_AUTO_TEST_CASE(EveryNameHasTheStandardsValue)
{
        static_assert(std::ranges::all_of(names, [](auto const& name) noexcept -> bool { return name.first == name.second; }));
        static_assert(xfs::perms::unknown.bits() == 0xFFFFU);
        static_assert(xfs::perms::mask.bits() == 07777U);
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
                if (fs::perms(ours) != theirs or ours.bits() != word or ours != xfs::perms::from_bits(static_cast<std::uint16_t>(word))) {
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
                BOOST_CHECK_EQUAL(p.size(), model.size());
                BOOST_CHECK_EQUAL(p.bits(), mask);
                BOOST_CHECK(not p.contains(beyond));
                BOOST_CHECK(not p[beyond]);

                // The complement stays within the sixteen bits, and its named flags are the rest of the twelve.
                auto const complement = ~p;
                BOOST_CHECK_EQUAL(complement.bits(), static_cast<std::uint16_t>(~mask));
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
        BOOST_CHECK(p.intersects(xfs::perms::owner_all));
        BOOST_CHECK(p.contains(fs::perms::group_write));
        BOOST_CHECK(xfs::perms::owner_read.is_subset_of(p));
}

// The unnamed bits are in the word and survive every operator, but iteration and size see the named flags alone.
BOOST_AUTO_TEST_CASE(UnnamedBitsAreKeptButNotIterated)
{
        auto const p = xfs::perms::unknown;
        BOOST_CHECK_EQUAL(p.size(), 12UZ);
        BOOST_CHECK(std::ranges::equal(p, ranks));
        BOOST_CHECK(p != xfs::perms::mask);
        BOOST_CHECK(~p == xfs::perms::none);
        BOOST_CHECK(~xfs::perms::none == xfs::perms::unknown);
        BOOST_CHECK((p - xfs::perms::mask).size() == 0UZ);
        BOOST_CHECK((p - xfs::perms::mask) != xfs::perms::none);
        BOOST_CHECK(std::ranges::equal(p - xfs::perms::mask, std::array<xfs::perm, 0>()));
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

// The flag type is a forward range of its rank enumeration, which yields each key by value.
BOOST_AUTO_TEST_CASE(TheFlagTypeIsAForwardRangeOfItsKeys)
{
        static_assert(std::forward_iterator<xfs::perms::iterator>);
        static_assert(std::ranges::forward_range<xfs::perms const>);
        static_assert(std::ranges::sized_range<xfs::perms const>);
        static_assert(std::same_as<std::iter_reference_t<xfs::perms::iterator>, xfs::perm>);

        auto const p = xfs::perms::owner_all;
        auto it      = p.begin();
        BOOST_CHECK(*it++ == xfs::perm::owner_exec);
        BOOST_CHECK(*it == xfs::perm::owner_write);
        BOOST_CHECK(++it != p.end());
        BOOST_CHECK(++it == p.end());
}

// The set prints as its keys do, through the rank enumeration's own formatter, in rank order.
BOOST_AUTO_TEST_CASE(TheFlagTypeFormatsItsFlagsByName)
{
        BOOST_CHECK_EQUAL(std::format("{}", xfs::perms::owner_read | xfs::perms::owner_write), "{owner_write, owner_read}");
        BOOST_CHECK_EQUAL(std::format("{}", xfs::perms::none), "{}");
        BOOST_CHECK_EQUAL(std::format("{}", xfs::perms::set_uid | xfs::perms::others_exec), "{others_exec, set_uid}");
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
        BOOST_CHECK(std::ranges::equal(m, std::array{mode::read, mode::exec}));
        BOOST_CHECK(m.contains(mode::exec));
        BOOST_CHECK(not m.contains(mode::write));
        BOOST_CHECK(not m.contains(std::bit_cast<mode>(std::uint8_t{0x08})));
        BOOST_CHECK(~m == modes(mode::write));
        BOOST_CHECK(~modes() == modes::from_bits(0x07));
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
        auto l         = lamps();
        l[mode::write] = true;
        BOOST_CHECK(l[mode::write] == true);
        BOOST_CHECK(l[mode::read] == false);
        BOOST_CHECK(true == l[mode::write]);
        l[mode::write] = false;
        BOOST_CHECK(l[mode::write] == false);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
