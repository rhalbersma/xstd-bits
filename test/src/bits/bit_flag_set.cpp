//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>           // for_each_type
#include <test/set/enums.hpp>               // perm
#include <test/set/lookup.hpp>              // lookup_mismatches
#include <xstd/bits/bit/bit_convert.hpp>    // bit_convert
#include <xstd/bits/bit_blocks.hpp>         // bit_fast, underlying_block_t
#include <xstd/bits/bit_enum_set.hpp>       // bit_enum_set
#include <xstd/bits/bit_fixed_set.hpp>      // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp>   // bit_flag_mapping
#include <xstd/bits/bit_flag_set.hpp>       // bit_flag_set
#include <xstd/bits/detail/set_adaptor.hpp> // disjoint, includes, intersects
#include <xstd/bits/from_blocks.hpp>        // from_blocks
#include <xstd/filesystem.hpp>              // perms
#include <xstd/ints/concepts/bit_mask.hpp>  // bit_mask
#include <boost/test/unit_test.hpp>         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                        // ranges::equal, ranges::includes, ranges::set_difference, ranges::set_intersection, ranges::set_symmetric_difference, ranges::set_union
#include <array>                            // array, to_array
#include <bit>                              // bit_cast, bit_floor, countr_zero, has_single_bit, popcount
#include <bitset>                           // bitset
#include <compare>                          // is_gt, is_lt
#include <concepts>                         // convertible_to, same_as
#include <cstddef>                          // size_t
#include <cstdint>                          // int64_t, int8_t, uint16_t, uint32_t, uint64_t, uint8_t
#include <filesystem>                       // exists, path, perm_options, permissions, perms, remove, status, temp_directory_path
#include <format>                           // format, format_to, formatter
#include <fstream>                          // ofstream
#include <functional>                       // greater, hash, ranges::greater
#include <initializer_list>                 // initializer_list
#include <ios>                              // ios_base
#include <iterator>                         // bidirectional_iterator, inserter, iter_reference_t, ranges::distance
#include <limits>                           // numeric_limits
#include <random>                           // random_device
#include <ranges>                           // bidirectional_range, iota, ranges::swap, sized_range, views::reverse
#include <set>                              // set
#include <sstream>                          // istringstream, ostringstream
#include <stdexcept>                        // out_of_range
#include <string_view>                      // string_view
#include <tuple>                            // tuple
#include <type_traits>                      // underlying_type_t
#include <utility>                          // pair, to_underlying
#include <vector>                           // vector

namespace {

namespace fs  = std::filesystem;
namespace xfs = xstd::filesystem;

// A program-defined bitmask enumeration with the operators [bitmask.types] asks for, three flags in eight bits.
enum class mode : std::uint8_t
{
        read  = 0x01,
        write = 0x02,
        exec  = 0x04,
};

[[nodiscard]] constexpr auto operator~(mode m) noexcept
        -> mode
{
        return std::bit_cast<mode>(static_cast<std::uint8_t>(~std::to_underlying(m)));
}

[[nodiscard]] constexpr auto operator&(mode a, mode b) noexcept
        -> mode
{
        return std::bit_cast<mode>(static_cast<std::uint8_t>(std::to_underlying(a) & std::to_underlying(b)));
}

[[nodiscard]] constexpr auto operator|(mode a, mode b) noexcept
        -> mode
{
        return std::bit_cast<mode>(static_cast<std::uint8_t>(std::to_underlying(a) | std::to_underlying(b)));
}

[[nodiscard]] constexpr auto operator^(mode a, mode b) noexcept
        -> mode
{
        return std::bit_cast<mode>(static_cast<std::uint8_t>(std::to_underlying(a) ^ std::to_underlying(b)));
}

constexpr auto operator&=(mode& a, mode b) noexcept
        -> mode&
{
        return a = a & b;
}

constexpr auto operator|=(mode& a, mode b) noexcept
        -> mode&
{
        return a = a | b;
}

constexpr auto operator^=(mode& a, mode b) noexcept
        -> mode&
{
        return a = a ^ b;
}

} // namespace

// A mode prints its name, so a set of them prints as the names of its flags.
template<>
struct std::formatter<mode> : std::formatter<std::string_view>
{
        // Pointers rather than string_view, whose conversion from a literal here crashes GCC 17 trunk.
        static constexpr std::array<char const*, 3> names = {"read", "write", "exec"};

        // A one-bit value, which a set of modes yields.
        template<class Context>
        [[nodiscard]] auto format(mode m, Context& ctx) const
        {
                return std::format_to(ctx.out(), "{}", names[static_cast<std::size_t>(std::countr_zero(std::to_underlying(m)))]);
        }
};

namespace {

// The three flags of mode, and every bit of its byte.
using modes      = xstd::bit_flag_set<mode, 3>;
using full_modes = xstd::bit_flag_set<mode>;

// Twelve bits wide, so a value coming in from the standard's type has no bit above set_uid.
using narrow_perms = xstd::bit_flag_set<fs::perms, 12>;

// A bitset mask, sixteen bits wide and twelve of sixteen.
using bitset_flags        = xstd::bit_flag_set<std::bitset<16>>;
using narrow_bitset_flags = xstd::bit_flag_set<std::bitset<16>, 12>;

// A user's own class over the flag type, which adds a name and inherits the rest, its operators returning the base.
class user_perms : public xfs::perms
{
public:
        using xfs::perms::perms;

        [[nodiscard]] constexpr auto is_private() const noexcept
                -> bool
        {
                return (*this & (fs::perms::group_all | fs::perms::others_all)) == fs::perms::none;
        }
};

// Every name [fs.enum.perms] lists, with its value.
constexpr auto names = std::to_array<std::pair<std::string_view, fs::perms>>({
        {"none", fs::perms::none},
        {"owner_read", fs::perms::owner_read},
        {"owner_write", fs::perms::owner_write},
        {"owner_exec", fs::perms::owner_exec},
        {"owner_all", fs::perms::owner_all},
        {"group_read", fs::perms::group_read},
        {"group_write", fs::perms::group_write},
        {"group_exec", fs::perms::group_exec},
        {"group_all", fs::perms::group_all},
        {"others_read", fs::perms::others_read},
        {"others_write", fs::perms::others_write},
        {"others_exec", fs::perms::others_exec},
        {"others_all", fs::perms::others_all},
        {"all", fs::perms::all},
        {"set_uid", fs::perms::set_uid},
        {"set_gid", fs::perms::set_gid},
        {"sticky_bit", fs::perms::sticky_bit},
        {"mask", fs::perms::mask},
        {"unknown", fs::perms::unknown},
});

// The value with bit i alone, past the named ones included, through bit_cast as no enumerator need name it.
[[nodiscard]] constexpr auto bit_at(std::size_t i) noexcept
        -> fs::perms
{
        return std::bit_cast<fs::perms>(static_cast<std::underlying_type_t<fs::perms>>(1U << i));
}

// The twelve permission bits, each a one-bit value of the standard's type.
constexpr auto ranks = [] noexcept -> std::array<fs::perms, 12> {
        auto nrv = std::array<fs::perms, 12>();
        for (auto const i : std::views::iota(0UZ, nrv.size())) {
                nrv[i] = bit_at(i);
        }
        return nrv;
}();

// The model orders its keys as the flag type does, the highest flag first.
using model_type = std::set<fs::perms, std::greater<>>;

// The flags a twelve-bit mask picks, as the flag type and as the model.
auto subset(std::size_t mask)
        -> std::pair<xfs::perms, model_type>
{
        auto nrv = std::pair<xfs::perms, model_type>();
        for (auto const k : ranks) {
                if ((mask & static_cast<std::size_t>(std::to_underlying(k))) != 0UZ) {
                        nrv.first |= k;
                        nrv.second.insert(k);
                }
        }
        return nrv;
}

auto with(model_type m, fs::perms k)
        -> model_type
{
        m.insert(k);
        return m;
}

auto without(model_type m, fs::perms k)
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
auto meets_the_standards_type(xfs::perms a, fs::perms b)
        -> void
{
        auto const mine = fs::perms(a);
        BOOST_CHECK((a | b) == (mine | b));
        BOOST_CHECK((b | a) == (b | mine));
        BOOST_CHECK((a & b) == (mine & b));
        BOOST_CHECK((b & a) == (b & mine));
        BOOST_CHECK((a ^ b) == (mine ^ b));
        BOOST_CHECK((b ^ a) == (b ^ mine));
        BOOST_CHECK((a - b) == (mine & ~b));
        BOOST_CHECK((b - a) == (b & ~mine));
        BOOST_CHECK((a == b) == (mine == b));
        BOOST_CHECK((b == a) == (b == mine));
}

// One key of a subset, probed by contains and bounded, then inserted and erased, against the model.
auto agrees_at_key(xfs::perms const& p, model_type const& model, fs::perms k)
        -> void
{
        BOOST_CHECK_EQUAL(p.contains(k), model.contains(k));
        BOOST_CHECK_EQUAL(xfs::perms(k).is_subset_of(p), model.contains(k));
        BOOST_CHECK_EQUAL(std::ranges::distance(p.begin(), p.lower_bound(k)), std::ranges::distance(model.begin(), model.lower_bound(k)));
        BOOST_CHECK_EQUAL(std::ranges::distance(p.begin(), p.upper_bound(k)), std::ranges::distance(model.begin(), model.upper_bound(k)));
        BOOST_CHECK_EQUAL((~p).contains(k), not model.contains(k));

        auto x = p;
        BOOST_CHECK_EQUAL(x.insert(k).second, not model.contains(k));
        BOOST_CHECK(std::ranges::equal(x, with(model, k)));
        BOOST_CHECK_EQUAL(x.erase(k), 1UZ);
        BOOST_CHECK(std::ranges::equal(x, without(model, k)));
}

// Values of the standard's type that are no key: none, several bits, and every bit.
constexpr auto non_keys = std::to_array<fs::perms>({fs::perms::none, fs::perms::owner_all, fs::perms::group_all, fs::perms::others_all, fs::perms::all, fs::perms::mask, fs::perms::unknown, fs::perms::owner_read | fs::perms::others_exec});

// One subset, walked both ways, counted and complemented, then each key and each value that is none against the model.
auto agrees_on_subset(std::size_t mask)
        -> void
{
        auto const [p, model] = subset(mask);
        BOOST_CHECK(std::ranges::equal(p, model));
        BOOST_CHECK(std::ranges::equal(std::views::reverse(p), std::views::reverse(model)));
        BOOST_CHECK_EQUAL(p.size(), model.size());
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(p), mask);

        // The complement is within the sixteen bits, the twelve permissions less these and the four bits above.
        auto const complement = ~p;
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(complement), static_cast<std::uint16_t>(~mask));
        BOOST_CHECK_EQUAL(complement.size(), 16UZ - model.size());
        for (auto const k : ranks) {
                agrees_at_key(p, model, k);
        }
        auto mismatches = 0UZ;
        for (auto const v : non_keys) {
                mismatches += test::set::lookup_mismatches(p, model, v);
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);
}

// Two subsets, queried and combined by each operator and its compound form, against std::set's algorithms.
auto agrees_on_pair(std::size_t lhs, std::size_t rhs)
        -> void
{
        auto const [a, ma] = subset(lhs);
        auto const [b, mb] = subset(rhs);
        BOOST_CHECK_EQUAL(b.is_subset_of(a), std::ranges::includes(ma, mb, std::ranges::greater()));
        BOOST_CHECK_EQUAL(a.is_subset_of(b), std::ranges::includes(mb, ma, std::ranges::greater()));
        BOOST_CHECK_EQUAL(intersects(a, b), not combined(ma, mb, std::ranges::set_intersection).empty());
        BOOST_CHECK_EQUAL(disjoint(a, b), not intersects(a, b));
        BOOST_CHECK_EQUAL(includes(a, b), std::ranges::includes(ma, mb, std::ranges::greater()));
        BOOST_CHECK_EQUAL(includes(a, b), b.is_subset_of(a));

        // The standard's value on either side converts, as it does for the operators.
        BOOST_CHECK_EQUAL(includes(a, fs::perms(b)), includes(a, b));
        BOOST_CHECK_EQUAL(includes(fs::perms(a), b), includes(a, b));
        BOOST_CHECK_EQUAL(disjoint(a, fs::perms(b)), disjoint(a, b));
        BOOST_CHECK_EQUAL(intersects(fs::perms(a), b), intersects(a, b));
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

// The words each 16-bit value is ordered against: the ends, single bits and runs, inside and above the twelve flags.
constexpr auto order_probes = std::to_array<std::uint16_t>({0x0000, 0x0001, 0x0007, 0x0100, 0x01FF, 0x0800, 0x0FFF, 0x1000, 0xF000, 0xFFFF});

// How many probes the word orders against otherwise than the enumeration: <=> both ways, and the mixed <.
auto order_mismatches(std::uint16_t word)
        -> std::size_t
{
        auto mismatches = 0UZ;
        auto const p    = xfs::perms(xstd::from_blocks, word);
        for (auto const other : order_probes) {
                auto const q        = xfs::perms(xstd::from_blocks, other);
                auto const expected = std::to_underlying(fs::perms(p)) <=> std::to_underlying(fs::perms(q));
                auto const mirrored = std::to_underlying(fs::perms(q)) <=> std::to_underlying(fs::perms(p));
                if ((p <=> q) != expected or (q <=> p) != mirrored or (p < fs::perms(q)) != std::is_lt(expected)) {
                        ++mismatches;
                }
        }
        return mismatches;
}

// How many probes a bitset flag value orders against otherwise than the numbers its bits spell.
auto bitset_order_mismatches(std::uint16_t word)
        -> std::size_t
{
        auto mismatches = 0UZ;
        auto const p    = bitset_flags(xstd::from_blocks, word);
        for (auto const other : order_probes) {
                auto const q = bitset_flags(xstd::from_blocks, other);
                if ((p <=> q) != (std::bitset<16>(p).to_ulong() <=> std::bitset<16>(q).to_ulong())) {
                        ++mismatches;
                }
        }
        return mismatches;
}

#ifdef __clang__

// Every standard name has a case; any other value, or an implementation's own enumerator, falls through to "".
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
#pragma clang diagnostic ignored "-Wswitch-default"

#endif

// The [fs.enum.perms] name of a value: a switch on the standard's type, or on the flag type through its conversion.
template<class Perms>
auto name_of(Perms p)
        -> std::string_view
{
        switch (p) {
                case fs::perms::none:
                        return "none";
                case fs::perms::owner_read:
                        return "owner_read";
                case fs::perms::owner_write:
                        return "owner_write";
                case fs::perms::owner_exec:
                        return "owner_exec";
                case fs::perms::owner_all:
                        return "owner_all";
                case fs::perms::group_read:
                        return "group_read";
                case fs::perms::group_write:
                        return "group_write";
                case fs::perms::group_exec:
                        return "group_exec";
                case fs::perms::group_all:
                        return "group_all";
                case fs::perms::others_read:
                        return "others_read";
                case fs::perms::others_write:
                        return "others_write";
                case fs::perms::others_exec:
                        return "others_exec";
                case fs::perms::others_all:
                        return "others_all";
                case fs::perms::all:
                        return "all";
                case fs::perms::set_uid:
                        return "set_uid";
                case fs::perms::set_gid:
                        return "set_gid";
                case fs::perms::sticky_bit:
                        return "sticky_bit";
                case fs::perms::mask:
                        return "mask";
                case fs::perms::unknown:
                        return "unknown";
        }
        return "";
}

#ifdef __clang__

#pragma clang diagnostic pop

#endif

// How many 16-bit values a set over the flag mapping converts, compares or combines otherwise than the standard's type.
template<class X>
auto conversion_mismatches()
        -> std::size_t
{
        constexpr auto used = static_cast<unsigned>((1UZ << X::max_size()) - 1UZ);
        auto mismatches     = 0UZ;
        for (auto const word : std::views::iota(0U, 0x10000U)) {
                auto const theirs = static_cast<fs::perms>(word);
                auto const low    = static_cast<fs::perms>(word & used);
                X const ours      = low;
                if (fs::perms(ours) != low or ours != low or (ours == theirs) != (low == theirs) or fs::perms(ours & theirs) != low or fs::perms(ours - theirs) != fs::perms::none or fs::perms(ours | low) != low or fs::perms(ours ^ low) != fs::perms::none) {
                        ++mismatches;
                }
        }
        return mismatches;
}

// Code written against std::filesystem::perms, run on a fresh file with only the variable's type spelled Perms.
template<class Perms>
auto adjust_permissions(fs::path const& path)
        -> std::vector<fs::perms>
{
        std::ofstream(path).put('x');
        auto trace = std::vector<fs::perms>();

        Perms p = fs::status(path).permissions();
        trace.push_back(p);
        p &= ~(fs::perms::group_write | fs::perms::others_write);
        if ((p & fs::perms::owner_read) == fs::perms::none) {
                p |= fs::perms::owner_read;
        }
        trace.push_back(p);
        fs::permissions(path, p);
        trace.push_back(fs::status(path).permissions());
        fs::permissions(path, fs::perms::owner_exec, fs::perm_options::add);
        trace.push_back(fs::status(path).permissions());
        fs::permissions(path, fs::perms::owner_all | fs::perms::group_read, fs::perm_options::replace);
        trace.push_back(fs::status(path).permissions());
        fs::permissions(path, fs::perms::owner_write, fs::perm_options::remove);
        trace.push_back(fs::status(path).permissions());

        // Writable again, since Windows will not remove a read-only file.
        fs::permissions(path, fs::perms::owner_all, fs::perm_options::add);
        fs::remove(path);
        return trace;
}

// Whether Mask keys a flag type at its default width.
template<class Mask>
concept names_a_flag_type = requires { typename xstd::bit_flag_set<Mask>; };

// Integer masks: each unsigned width, int and std::int8_t below their sign bits, and twelve of sixteen bits.
using integer_flags = std::tuple<xstd::bit_flag_set<std::uint8_t>, xstd::bit_flag_set<std::uint16_t>, xstd::bit_flag_set<std::uint32_t>, xstd::bit_flag_set<std::uint64_t>, xstd::bit_flag_set<int>, xstd::bit_flag_set<std::int8_t>, xstd::bit_flag_set<std::uint16_t, 12>>;

// The unsigned word an integer flag type's mask is read as.
template<class X>
using word_t = xstd::underlying_block_t<typename X::key_type>;

// The word with every bit below the flag type's width set.
template<class X>
[[nodiscard]] constexpr auto low_bits() noexcept
        -> word_t<X>
{
        using word_type = word_t<X>;
        return static_cast<word_type>(std::numeric_limits<word_type>::max() >> (static_cast<std::size_t>(std::numeric_limits<word_type>::digits) - X::max_size()));
}

// Words below the width: every one up to sixteen bits, else none, all, two stripes, and each single and adjacent pair.
template<class X>
auto probe_words()
        -> std::vector<word_t<X>>
{
        using word_type  = word_t<X>;
        constexpr auto N = X::max_size();
        auto nrv         = std::vector<word_type>();
        if constexpr (N <= 16UZ) {
                for (auto const w : std::views::iota(0UZ, 1UZ << N)) {
                        nrv.push_back(static_cast<word_type>(w));
                }
        } else {
                constexpr auto low    = low_bits<X>();
                constexpr auto stripe = static_cast<word_type>(low / 3U);
                nrv.insert(nrv.end(), {word_type{}, low, stripe, static_cast<word_type>(low ^ stripe)});
                for (auto const i : std::views::iota(0UZ, N)) {
                        nrv.push_back(static_cast<word_type>(word_type{1} << i));
                }
                for (auto const i : std::views::iota(1UZ, N)) {
                        nrv.push_back(static_cast<word_type>(word_type{3} << (i - 1UZ)));
                }
        }
        return nrv;
}

// How many checks one word of an integer flag type fails: the conversions, the walk from the highest flag, the lookups.
template<class X>
auto word_mismatches(word_t<X> w)
        -> std::size_t
{
        using mask_type = X::key_type;
        using word_type = word_t<X>;
        auto const m    = static_cast<mask_type>(w);
        X const x       = m;
        auto mismatches = 0UZ;
        if (mask_type(x) != m or x != m or X(xstd::from_blocks, w) != m or x.size() != static_cast<std::size_t>(std::popcount(w))) {
                ++mismatches;
        }

        // From the highest flag down, each one bit of the word, which together spell it.
        auto rest = w;
        for (auto const k : x) {
                auto const bit = static_cast<word_type>(mask_type(k));
                if (not std::has_single_bit(bit) or bit != std::bit_floor(rest) or not x.contains(k)) {
                        ++mismatches;
                }
                rest = static_cast<word_type>(rest ^ bit);
        }

        // A value of several bits, or of none, is no key even where each of its bits is an element.
        if (x.contains(m) != std::has_single_bit(w) or x.count(m) != (std::has_single_bit(w) ? 1UZ : 0UZ) or rest != word_type{}) {
                ++mismatches;
        }
        return mismatches;
}

// How many operators an integer flag type and its mask, on either side, answer otherwise than the mask itself does.
template<class X>
auto operator_mismatches(word_t<X> w, typename X::key_type b)
        -> std::size_t
{
        using mask_type = X::key_type;
        using word_type = word_t<X>;
        auto const m    = static_cast<mask_type>(w);
        auto const v    = static_cast<word_type>(b);
        X const x       = m;
        auto mismatches = 0UZ;
        // The mask's own answers, worked in its unsigned word so that a signed mask's bits are read as bits.
        auto const both   = static_cast<mask_type>(static_cast<word_type>(w | v));
        auto const common = static_cast<mask_type>(static_cast<word_type>(w & v));
        auto const either = static_cast<mask_type>(static_cast<word_type>(w ^ v));
        auto const m_only = static_cast<mask_type>(static_cast<word_type>(w & static_cast<word_type>(~v)));
        auto const b_only = static_cast<mask_type>(static_cast<word_type>(v & static_cast<word_type>(~w)));
        mismatches += static_cast<std::size_t>(mask_type(x | b) != both or mask_type(b | x) != both);
        mismatches += static_cast<std::size_t>(mask_type(x & b) != common or mask_type(b & x) != common);
        mismatches += static_cast<std::size_t>(mask_type(x ^ b) != either or mask_type(b ^ x) != either);
        mismatches += static_cast<std::size_t>(mask_type(x - b) != m_only or mask_type(b - x) != b_only);
        mismatches += static_cast<std::size_t>((x == b) != (m == b) or (x <=> X(b)) != (m <=> b));
        mismatches += static_cast<std::size_t>(includes(x, b) != ((w & v) == v) or intersects(x, b) != ((w & v) != word_type{}));
        return mismatches;
}

// How many checks an integer flag type fails over its probe words, against none, every bit, a stripe and either end.
template<class X>
auto integer_mismatches()
        -> std::size_t
{
        using mask_type   = X::key_type;
        using word_type   = word_t<X>;
        auto const low    = low_bits<X>();
        auto const others = std::to_array<mask_type>({mask_type{}, static_cast<mask_type>(low), static_cast<mask_type>(low / 3U), mask_type{1}, static_cast<mask_type>(word_type{1} << (X::max_size() - 1UZ))});
        auto mismatches   = 0UZ;
        for (auto const w : probe_words<X>()) {
                mismatches += word_mismatches<X>(w);
                for (auto const b : others) {
                        mismatches += operator_mismatches<X>(w, b);
                }
        }
        return mismatches;
}

// How many of a stream type's constants a flag type of it converts, counts or looks up otherwise than the mask does.
template<class Mask>
auto ios_mismatches(std::initializer_list<Mask> constants)
        -> std::size_t
{
        using X         = xstd::bit_flag_set<Mask>;
        using word_type = xstd::underlying_block_t<Mask>;
        auto mismatches = 0UZ;
        for (auto const c : constants) {
                X const x    = c;
                auto const w = static_cast<word_type>(c);
                if (Mask(x) != c or x != c or x.size() != static_cast<std::size_t>(std::popcount(w)) or x.contains(c) != std::has_single_bit(w) or not includes(x, c)) {
                        ++mismatches;
                }
                for (auto const k : x) {
                        auto const bit = static_cast<word_type>(Mask(k));
                        if (not std::has_single_bit(bit) or (bit & w) == word_type{}) {
                                ++mismatches;
                        }
                }
        }
        return mismatches;
}

} // namespace

BOOST_AUTO_TEST_SUITE(BitFlagSet)

// The standard's type and every flag type meet [bitmask.types].
BOOST_AUTO_TEST_CASE(EveryFlagTypeIsABitmaskType)
{
        static_assert(xstd::bit_mask<fs::perms> and xstd::bit_mask<mode> and xstd::bit_mask<std::bitset<16>>);
        static_assert(xstd::bit_mask<xfs::perms> and xstd::bit_mask<narrow_perms>);
        static_assert(xstd::bit_mask<modes> and xstd::bit_mask<full_modes>);
        static_assert(xstd::bit_mask<bitset_flags> and xstd::bit_mask<narrow_bitset_flags>);
        static_assert(xstd::bit_convert<std::uint16_t>(xfs::perms{}) == 0U and xstd::bit_convert<std::uint16_t>(xfs::perms(fs::perms::none)) == 0U);

        BOOST_CHECK(true);
}

// The flag type is the fixed set of the mask's one-bit values over the flag mapping, in the least block holding N.
BOOST_AUTO_TEST_CASE(TheFlagTypeIsTheFixedSetOverTheFlagMapping)
{
        static_assert(std::same_as<xfs::perms, xstd::basic_bit_fixed_set<fs::perms, std::uint16_t, 16, xstd::bit_flag_mapping<fs::perms, 16>, std::greater<fs::perms>>>);                 // NOLINT(modernize-use-transparent-functors): the comparator the alias names
        static_assert(std::same_as<modes, xstd::basic_bit_fixed_set<mode, std::uint8_t, 3, xstd::bit_flag_mapping<mode, 3>, std::greater<mode>>>);                                        // NOLINT(modernize-use-transparent-functors): the comparator the alias names
        static_assert(std::same_as<bitset_flags, xstd::basic_bit_fixed_set<std::bitset<16>, std::uint16_t, 16, xstd::bit_flag_mapping<std::bitset<16>>, std::greater<std::bitset<16>>>>); // NOLINT(modernize-use-transparent-functors): the comparator the alias names

        BOOST_CHECK(true);
}

// Only a set over a mask mapping converts with its key: a set of positions or of listed enumerators does not.
BOOST_AUTO_TEST_CASE(OnlyAMaskMappingMakesASetConvertWithItsKey)
{
        static_assert(std::convertible_to<fs::perms, xfs::perms> and std::convertible_to<xfs::perms, fs::perms>);
        static_assert(not std::convertible_to<std::size_t, xstd::bit_fixed_set<8>> and not std::convertible_to<xstd::bit_fixed_set<8>, std::size_t>);
        static_assert(not std::convertible_to<test::set::perm, xstd::bit_enum_set<test::set::perm>>);
        static_assert(not std::convertible_to<xstd::bit_enum_set<test::set::perm>, test::set::perm>);

        // A listed enumerator is the one key it is, so | inserts it and & keeps it alone.
        auto const s = xstd::bit_enum_set<test::set::perm>{test::set::perm::read};
        BOOST_CHECK((s | test::set::perm::exec).size() == 2UZ and (s & test::set::perm::read).size() == 1UZ);
}

// Any block and any order over the flag mapping convert alike: two blocks of a byte, one wider block, ascending keys.
BOOST_AUTO_TEST_CASE(EveryBlockOverTheFlagMappingConvertsAlike)
{
        using byte_blocks        = xstd::basic_bit_fixed_set<fs::perms, std::uint8_t, 16, xstd::bit_flag_mapping<fs::perms, 16>, std::greater<fs::perms>>; // NOLINT(modernize-use-transparent-functors): the flag type's comparator
        using narrow_byte_blocks = xstd::basic_bit_fixed_set<fs::perms, std::uint8_t, 12, xstd::bit_flag_mapping<fs::perms, 12>>;
        using wide_block         = xstd::basic_bit_fixed_set<fs::perms, std::uint64_t, 12, xstd::bit_flag_mapping<fs::perms, 12>>;
        static_assert(sizeof(byte_blocks) == 2UZ and sizeof(xstd::bit_fast<xfs::perms>) == sizeof(std::uint_fast16_t));
        static_assert(fs::perms(byte_blocks(fs::perms::unknown)) == fs::perms::unknown and byte_blocks{fs::perms::owner_all}.size() == 3UZ);
        BOOST_CHECK_EQUAL(conversion_mismatches<xfs::perms>(), 0UZ);
        BOOST_CHECK_EQUAL(conversion_mismatches<byte_blocks>(), 0UZ);
        BOOST_CHECK_EQUAL(conversion_mismatches<narrow_byte_blocks>(), 0UZ);
        BOOST_CHECK_EQUAL(conversion_mismatches<wide_block>(), 0UZ);
        BOOST_CHECK_EQUAL(conversion_mismatches<xstd::bit_fast<xfs::perms>>(), 0UZ);
        BOOST_CHECK(std::ranges::equal(narrow_byte_blocks(fs::perms::owner_read | fs::perms::set_gid), std::array{fs::perms::owner_read, fs::perms::set_gid}));
}

// The keys are the mask's one-bit values below N, by bit_flag_mapping, and key_compare is std::greater of the mask.
BOOST_AUTO_TEST_CASE(TheKeysAreTheMasksOneBitValues)
{
        static_assert(std::same_as<xfs::perms::key_type, fs::perms>);
        static_assert(std::same_as<xfs::perms::key_mapping_type, xstd::bit_flag_mapping<fs::perms, 16>>);
        static_assert(std::same_as<xfs::perms::key_compare, std::greater<fs::perms>>);
        static_assert(std::same_as<bitset_flags::key_type, std::bitset<16>>);
        static_assert(std::same_as<bitset_flags::key_compare, std::greater<std::bitset<16>>>);
        static_assert(xfs::perms::max_size() == 16UZ and narrow_perms::max_size() == 12UZ and modes::max_size() == 3UZ);
        static_assert(sizeof(xfs::perms) == sizeof(std::uint16_t) and sizeof(modes) == sizeof(std::uint8_t));

        BOOST_CHECK(true);
}

// Each name converts to the flag type and back with its value, in a constant expression and at run time.
BOOST_AUTO_TEST_CASE(EveryNameRoundTrips)
{
        static_assert(xstd::bit_convert<std::uint16_t>(xfs::perms(fs::perms::unknown)) == 0xFFFFU);
        static_assert(xstd::bit_convert<std::uint16_t>(xfs::perms(fs::perms::mask)) == 07777U);
        static_assert(xfs::perms(fs::perms::all) == fs::perms::all);
        for (auto const& [name, theirs] : names) {
                xfs::perms const ours = theirs;
                BOOST_CHECK(ours == theirs);
                BOOST_CHECK(fs::perms(ours) == theirs);
                BOOST_CHECK_EQUAL(name_of(ours), name);
                BOOST_CHECK_EQUAL(name_of(theirs), name);
        }
        BOOST_CHECK_EQUAL(name_of(xfs::perms(fs::perms::owner_read | fs::perms::group_read)), "");
}

// Every 16-bit value converts in from the standard's type and back unchanged.
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

// A braced list is the union of its values: one value is the conversion, and one-bit values are the set of them.
BOOST_AUTO_TEST_CASE(ABracedListIsTheUnionOfItsValues)
{
        static_assert(xfs::perms{fs::perms::owner_all} == fs::perms::owner_all);
        static_assert(xfs::perms{fs::perms::owner_read, fs::perms::group_read} == (fs::perms::owner_read | fs::perms::group_read));
        static_assert(xfs::perms{fs::perms::owner_all, fs::perms::group_read}.size() == 4UZ);
        auto const keys = std::vector{fs::perms::others_exec, fs::perms::owner_read};
        BOOST_CHECK((xfs::perms(keys.begin(), keys.end()) == (fs::perms::owner_read | fs::perms::others_exec)));
        BOOST_CHECK((xfs::perms(std::from_range, keys) == (fs::perms::owner_read | fs::perms::others_exec)));
        BOOST_CHECK(xfs::perms{}.empty());
}

// Each operator takes the standard's type on either side, and answers as the standard's own operator does.
BOOST_AUTO_TEST_CASE(EachOperatorMeetsTheStandardsTypeInBothOrders)
{
        auto const p = xfs::perms(fs::perms::owner_read);
        static_assert(std::same_as<decltype(p | fs::perms::owner_write), xfs::perms>);
        static_assert(std::same_as<decltype(fs::perms::owner_write | p), xfs::perms>);
        static_assert(std::same_as<decltype(p & fs::perms::owner_write), xfs::perms>);
        static_assert(std::same_as<decltype(fs::perms::owner_write & p), xfs::perms>);
        static_assert(std::same_as<decltype(p ^ fs::perms::owner_write), xfs::perms>);
        static_assert(std::same_as<decltype(fs::perms::owner_write ^ p), xfs::perms>);
        static_assert(std::same_as<decltype(p - fs::perms::owner_write), xfs::perms>);
        static_assert(std::same_as<decltype(fs::perms::owner_write - p), xfs::perms>);
        static_assert(std::same_as<decltype(p == fs::perms::owner_write), bool>);
        static_assert(std::same_as<decltype(fs::perms::owner_write == p), bool>);
        static_assert(std::same_as<decltype(fs::perms::owner_write != p), bool>);
        static_assert(std::same_as<decltype(p | p), xfs::perms> and std::same_as<decltype(~p), xfs::perms>);

        for (auto const& [a_name, a] : names) {
                for (auto const& [b_name, b] : names) {
                        meets_the_standards_type(a, b);
                }
        }
}

// A multi-bit value is a mask on either side, not one key: owner_all meets all three owner flags.
BOOST_AUTO_TEST_CASE(AMultiBitValueIsAMaskNotAKey)
{
        auto const p = xfs::perms(fs::perms::owner_read | fs::perms::group_all);
        BOOST_CHECK((p | fs::perms::owner_all) == (fs::perms::owner_all | fs::perms::group_all));
        BOOST_CHECK((fs::perms::owner_all & p) == fs::perms::owner_read);
        BOOST_CHECK((p ^ fs::perms::owner_all) == (fs::perms::owner_write | fs::perms::owner_exec | fs::perms::group_all));
        BOOST_CHECK((fs::perms::owner_all - p) == (fs::perms::owner_write | fs::perms::owner_exec));
        BOOST_CHECK(xfs::perms(fs::perms::group_all).is_subset_of(p) and not xfs::perms(fs::perms::owner_all).is_subset_of(p));
        BOOST_CHECK(xfs::perms(fs::perms::none).is_subset_of(p));
        BOOST_CHECK(intersects(p, xfs::perms(fs::perms::owner_all)));

        // As a key it names no flag, so the set holds no such element, and erasing it leaves the set as it was.
        auto x = p;
        BOOST_CHECK(not x.contains(fs::perms::group_all) and x.count(fs::perms::group_all) == 0UZ); // NOLINT(readability-container-contains): count is the member under test
        BOOST_CHECK(x.find(fs::perms::owner_all) == x.end());                                       // NOLINT(readability-container-contains): find is the member under test
        BOOST_CHECK(x.erase(fs::perms::owner_all) == 0UZ and x == p);
        BOOST_CHECK(not x.contains(fs::perms::none) and x.erase(fs::perms::none) == 0UZ and x == p);
}

// & and - read the standard's value below sixteen bits, where it can meet a flag, and == finds higher bits unequal.
BOOST_AUTO_TEST_CASE(AndMinusAndEqualityTakeAValueWithHigherBits)
{
        auto const p = xfs::perms(fs::perms::owner_all | fs::perms::group_write);
        BOOST_CHECK((p & ~fs::perms::group_write) == fs::perms::owner_all);
        BOOST_CHECK((~fs::perms::group_write & p) == fs::perms::owner_all);
        BOOST_CHECK((p - ~fs::perms::none) == fs::perms::none);
        BOOST_CHECK((p - ~fs::perms::owner_all) == fs::perms::owner_all);
        BOOST_CHECK((xfs::perms(fs::perms::unknown) & ~fs::perms::none) == fs::perms::unknown);
        BOOST_CHECK(not(xfs::perms(fs::perms::unknown) == ~fs::perms::none));
        BOOST_CHECK(not(~fs::perms::none == xfs::perms(fs::perms::unknown)));
        BOOST_CHECK(xfs::perms(fs::perms::unknown) != ~fs::perms::none);

        // The compound forms, against the standard's own on the same values.
        auto x      = p;
        auto theirs = fs::perms(p);
        x &= ~fs::perms::group_write;
        theirs &= ~fs::perms::group_write;
        BOOST_CHECK(x == theirs);
        BOOST_CHECK(&(x -= ~fs::perms::owner_all) == &x);
        BOOST_CHECK(x == fs::perms::owner_all);
        BOOST_CHECK(&(x |= fs::perms::group_read) == &x and x == (fs::perms::owner_all | fs::perms::group_read));
        BOOST_CHECK(&(x ^= fs::perms::owner_all) == &x and x == fs::perms::group_read);
        static_assert(std::same_as<decltype(x &= ~fs::perms::none), xfs::perms&>);
        static_assert(std::same_as<decltype(x -= ~fs::perms::none), xfs::perms&>);
        static_assert(std::same_as<decltype(x |= fs::perms::none), xfs::perms&>);
        static_assert(std::same_as<decltype(x ^= fs::perms::none), xfs::perms&>);
}

// The same code, once against the standard's type and once with the variable's type respelled, leaves a file the same.
BOOST_AUTO_TEST_CASE(RespelledCodeBehavesAsTheStandardsTypeOnARealFile)
{
        auto const path       = fs::temp_directory_path() / std::format("xstd-bits-bit-flag-set-{}.tmp", std::random_device()());
        auto const std_trace  = adjust_permissions<fs::perms>(path);
        auto const xstd_trace = adjust_permissions<xfs::perms>(path);
        BOOST_CHECK(std_trace == xstd_trace);
        BOOST_CHECK(not fs::exists(path));
}

// Every subset of the twelve permissions, walked, counted, probed, complemented and written through p[k].
BOOST_AUTO_TEST_CASE(EverySubsetAgreesWithStdSet)
{
        for (auto const mask : std::views::iota(0UZ, 1UZ << ranks.size())) {
                agrees_on_subset(mask);
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

// The four bits above the permissions are keys like the others: unknown holds all sixteen.
BOOST_AUTO_TEST_CASE(TheHighBitsAreKeysToo)
{
        auto const unknown = xfs::perms(fs::perms::unknown);
        BOOST_CHECK_EQUAL(unknown.size(), 16UZ);
        BOOST_CHECK(*unknown.begin() == bit_at(15UZ) and *unknown.rbegin() == fs::perms::others_exec);
        BOOST_CHECK(~unknown == fs::perms::none);
        BOOST_CHECK((unknown - fs::perms::mask).size() == 4UZ);
        BOOST_CHECK(std::is_lt(xfs::perms(fs::perms::mask) <=> unknown));
        xfs::perms const back = fs::perms(unknown);
        BOOST_CHECK(back == unknown);
}

// insert refuses a one-bit value at or above N, as a set refuses a key past its max_size(), and the word is untouched.
BOOST_AUTO_TEST_CASE(InsertingAPositionAtOrAboveTheWidthThrows)
{
        auto p = narrow_perms(fs::perms::owner_read);
        BOOST_CHECK_THROW(p.insert(bit_at(12UZ)), std::out_of_range);
        BOOST_CHECK(p == fs::perms::owner_read);
        BOOST_CHECK(not p.contains(bit_at(12UZ)));
}

// The set reading's members come with the base: insert, erase, find, the reverse range, clear, swap and hash.
BOOST_AUTO_TEST_CASE(TheInheritedSetInterfaceWorks)
{
        auto p = xfs::perms(xstd::from_blocks, std::uint16_t{0xF000});
        BOOST_CHECK(p.insert(fs::perms::owner_read).second);
        BOOST_CHECK(not p.insert(fs::perms::owner_read).second);
        p.insert(fs::perms::group_exec);
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(p), 0xF108U);
        BOOST_CHECK(*p.find(fs::perms::group_exec) == fs::perms::group_exec);
        BOOST_CHECK(p.find(fs::perms::others_read) == p.end()); // NOLINT(readability-container-contains): find is the inherited member under test
        BOOST_CHECK(*p.rbegin() == fs::perms::group_exec);
        BOOST_CHECK(std::ranges::equal(std::views::reverse(p) | std::views::take(2), std::array{fs::perms::group_exec, fs::perms::owner_read}));
        BOOST_CHECK_EQUAL(p.erase(fs::perms::owner_read), 1UZ);
        BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint16_t>(p), 0xF008U);

        auto q = xfs::perms(fs::perms::owner_all);
        swap(p, q);
        BOOST_CHECK(p == fs::perms::owner_all and xstd::bit_convert<std::uint16_t>(q) == 0xF008U);
        std::ranges::swap(p, q);
        BOOST_CHECK(q == fs::perms::owner_all);
        BOOST_CHECK_EQUAL(std::hash<xfs::perms>()(q), std::hash<xfs::perms>()(xfs::perms(fs::perms::owner_all)));
        p.clear();
        BOOST_CHECK(p == fs::perms::none);
}

// contains is std::set's membership of one flag; all-of is includes, any-of intersects and none-of disjoint.
BOOST_AUTO_TEST_CASE(ContainsIsMembershipOfOneFlag)
{
        auto p = xfs::perms(fs::perms::owner_read | fs::perms::group_write);
        static_assert(std::same_as<decltype(p.contains(fs::perms::owner_read)), bool>);
        BOOST_CHECK(p.contains(fs::perms::owner_read) and not p.contains(fs::perms::owner_write));
        BOOST_CHECK(xfs::perms(fs::perms::owner_read).is_subset_of(p) == p.contains(fs::perms::owner_read));
        BOOST_CHECK(not xfs::perms(fs::perms::owner_all).is_subset_of(p) and intersects(p, xfs::perms(fs::perms::owner_all)));
        BOOST_CHECK(not p.contains(fs::perms::owner_all) and not includes(p, fs::perms::owner_all) and intersects(p, fs::perms::owner_all));
        BOOST_CHECK(includes(p, fs::perms::owner_read | fs::perms::group_write) and includes(fs::perms::all, p));
        BOOST_CHECK(disjoint(p, fs::perms::others_all) and not disjoint(fs::perms::group_all, p));
        static_assert(noexcept(includes(p, p)) and noexcept(disjoint(p, p)) and noexcept(intersects(p, p)));
        p.insert(fs::perms::owner_write);
        BOOST_CHECK(p == (fs::perms::owner_read | fs::perms::owner_write | fs::perms::group_write));
        p.erase(fs::perms::owner_read);
        BOOST_CHECK(p == (fs::perms::owner_write | fs::perms::group_write));
}

// The flag type is a bidirectional range of the mask's one-bit values, whose read-only proxy converts to each.
BOOST_AUTO_TEST_CASE(TheFlagTypeIsABidirectionalRangeOfItsKeys)
{
        static_assert(std::bidirectional_iterator<xfs::perms::iterator>);
        static_assert(std::ranges::bidirectional_range<xfs::perms const>);
        static_assert(std::ranges::sized_range<xfs::perms const>);
        static_assert(std::same_as<std::iter_reference_t<xfs::perms::iterator>, xfs::perms::const_reference>);
        static_assert(std::convertible_to<xfs::perms::const_reference, fs::perms>);

        auto const p = xfs::perms(fs::perms::owner_all);
        auto it      = p.begin();
        BOOST_CHECK(*it++ == fs::perms::owner_read);
        BOOST_CHECK(*it == fs::perms::owner_write);
        BOOST_CHECK(++it != p.end());
        BOOST_CHECK(++it == p.end());
}

// The walk runs from the highest flag down, as ls -l reads the mode: owner_read first and others_exec last.
BOOST_AUTO_TEST_CASE(TheFlagTypeIteratesFromTheHighestFlagDown)
{
        constexpr auto rwx = std::to_array({fs::perms::owner_read, fs::perms::owner_write, fs::perms::owner_exec, fs::perms::group_read, fs::perms::group_write, fs::perms::group_exec, fs::perms::others_read, fs::perms::others_write, fs::perms::others_exec});
        BOOST_CHECK(std::ranges::equal(xfs::perms(fs::perms::all), rwx));
        BOOST_CHECK(xfs::perms(fs::perms::mask).front() == fs::perms::set_uid and xfs::perms(fs::perms::mask).back() == fs::perms::others_exec);
}

// <=> on two flag values is the enumeration's on their underlying words, and the mixed < through the conversion agrees.
BOOST_AUTO_TEST_CASE(ThreeWayComparisonIsTheEnumerations)
{
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0U, 0x10000U)) {
                mismatches += order_mismatches(static_cast<std::uint16_t>(word));
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);
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

// A program-defined mask: the flags of three bits or of the whole byte, printed through the mask's own formatter.
BOOST_AUTO_TEST_CASE(AProgramDefinedMaskKeysAFlagType)
{
        auto const m = modes{mode::read, mode::exec};
        BOOST_CHECK(std::ranges::equal(m, std::array{mode::exec, mode::read}));
        BOOST_CHECK(m.contains(mode::exec) and not m.contains(mode::write));
        BOOST_CHECK(modes(mode::exec | mode::read).is_subset_of(m) and not modes(mode::exec | mode::write).is_subset_of(m));
        BOOST_CHECK(~m == mode::write);
        BOOST_CHECK(mode(~modes()) == (mode::read | mode::write | mode::exec));
        BOOST_CHECK_EQUAL(std::format("{}", m), "{exec, read}");
        BOOST_CHECK_EQUAL(std::format("{}", modes()), "{}");

        // The mask's own operators, against which the flag type's are checked.
        auto bits = mode::read;
        bits |= mode::write;
        bits &= ~mode::read;
        bits ^= mode::exec;
        BOOST_CHECK(bits == (mode::write | mode::exec) and modes(bits) == (modes{mode::write} | mode::exec));

        // The whole byte: its complement holds the five bits no enumerator names.
        auto const f = full_modes(mode::write);
        BOOST_CHECK_EQUAL((~f).size(), 7UZ);
        BOOST_CHECK(std::to_underlying(mode(~f)) == 0xFDU);
        BOOST_CHECK((f | mode::read) == (mode::read | mode::write));
}

// A user's class over the flag type inherits its constructors and set interface, and adds a name of its own.
BOOST_AUTO_TEST_CASE(AUserMayDeriveAClassOfTheirOwn)
{
        auto const p = user_perms(fs::perms::owner_all);
        BOOST_CHECK(p.is_private());
        BOOST_CHECK(not user_perms(fs::perms::all).is_private());
        BOOST_CHECK((user_perms{fs::perms::owner_read, fs::perms::group_read}).size() == 2UZ);
        static_assert(std::same_as<decltype(p | fs::perms::group_read), xfs::perms>);
        BOOST_CHECK((p | fs::perms::group_read).size() == 4UZ);
}

// A bitset mask converts both ways at every 16-bit value, and its flags are one-bit bitsets, the highest first.
BOOST_AUTO_TEST_CASE(ABitsetMaskConvertsAndIterates)
{
        static_assert(std::bitset<16>(bitset_flags(std::bitset<16>(0x0123))) == std::bitset<16>(0x0123));
        static_assert(bitset_flags(std::bitset<16>(0x0123)) == std::bitset<16>(0x0123));
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0U, 0x10000U)) {
                auto const bits         = std::bitset<16>(word);
                bitset_flags const ours = bits;
                if (std::bitset<16>(ours) != bits or xstd::bit_convert<std::uint16_t>(ours) != word or ours != bits) {
                        ++mismatches;
                }
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);

        auto const p = bitset_flags(std::bitset<16>(0x0105));
        BOOST_CHECK((std::vector<std::bitset<16>>(p.begin(), p.end()) == std::vector{std::bitset<16>(0x0100), std::bitset<16>(0x0004), std::bitset<16>(0x0001)}));
        BOOST_CHECK(bitset_flags(std::bitset<16>(0x0101)).is_subset_of(p) and not bitset_flags(std::bitset<16>(0x0003)).is_subset_of(p));
        BOOST_CHECK(p.contains(std::bitset<16>(0x0004)) and not p.contains(std::bitset<16>(0x0002)));

        // A bitset of several bits, or of none, is no key: no element, and nothing to erase.
        auto x = p;
        BOOST_CHECK(not x.contains(std::bitset<16>(0x0005)) and x.count(std::bitset<16>(0x0005)) == 0UZ); // NOLINT(readability-container-contains): count is the member under test
        BOOST_CHECK(x.find(std::bitset<16>()) == x.end());                                                // NOLINT(readability-container-contains): find is the member under test
        BOOST_CHECK(x.erase(std::bitset<16>(0x0105)) == 0UZ and x == p);
        BOOST_CHECK(includes(p, std::bitset<16>(0x0005)) and disjoint(p, std::bitset<16>(0x0002)));
}

// Each operator takes the bitset on either side, as std::bitset's own do.
BOOST_AUTO_TEST_CASE(EachOperatorMeetsABitsetInBothOrders)
{
        auto const q = bitset_flags(xstd::from_blocks, std::uint16_t{0x0F0});
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(q | std::bitset<16>(0x00F)) == 0x0FF and xstd::bit_convert<std::uint16_t>(std::bitset<16>(0x00F) | q) == 0x0FF);
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(q & std::bitset<16>(0x030)) == 0x030 and xstd::bit_convert<std::uint16_t>(std::bitset<16>(0x030) & q) == 0x030);
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(q ^ std::bitset<16>(0x0FF)) == 0x00F and xstd::bit_convert<std::uint16_t>(std::bitset<16>(0x0FF) ^ q) == 0x00F);
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(q - std::bitset<16>(0x030)) == 0x0C0 and xstd::bit_convert<std::uint16_t>(std::bitset<16>(0x0FF) - q) == 0x00F);
        BOOST_CHECK(std::bitset<16>(0x0F0) == q and q != std::bitset<16>(0x0F1));

        auto x = q;
        BOOST_CHECK(x.insert(std::bitset<16>(0x001)).second);
        BOOST_CHECK(x == std::bitset<16>(0x0F1) and x.erase(std::bitset<16>(0x010)) == 1UZ);
        BOOST_CHECK(x.insert(std::bitset<16>(0x8000)).second and x.size() == 5UZ);
}

// <=> on two bitset flag values is the order of the numbers their bits spell.
BOOST_AUTO_TEST_CASE(ThreeWayComparisonOfABitsetIsNumeric)
{
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0U, 0x10000U)) {
                mismatches += bitset_order_mismatches(static_cast<std::uint16_t>(word));
        }
        BOOST_CHECK_EQUAL(mismatches, 0UZ);
}

// Against a sixteen-bit bitset, a twelve-bit flag type reads the low twelve, and finds any higher position unequal.
BOOST_AUTO_TEST_CASE(ANarrowerWidthReadsTheLowPositionsOfABitset)
{
        auto const high = std::bitset<16>(0x1001);
        auto const p    = narrow_bitset_flags(xstd::from_blocks, std::uint16_t{0x0FFF});
        BOOST_CHECK(std::bitset<16>(p & high) == std::bitset<16>(0x001));
        BOOST_CHECK(std::bitset<16>(p - high) == std::bitset<16>(0xFFE));
        BOOST_CHECK(not(narrow_bitset_flags(xstd::from_blocks, std::uint16_t{0x001}) == high));
        BOOST_CHECK(narrow_bitset_flags(xstd::from_blocks, std::uint16_t{0x001}) == std::bitset<16>(0x001));
        BOOST_CHECK(std::bitset<16>(~narrow_bitset_flags()) == std::bitset<16>(0x0FFF));
        BOOST_CHECK(intersects(p, narrow_bitset_flags(std::bitset<16>(0x001))));
}

// An integer type is a bitmask type, [bitmask.types]/1, and keys a flag type below its sign bit; bool and char do not.
BOOST_AUTO_TEST_CASE(AnIntegerMaskKeysAFlagTypeBelowItsSignBit)
{
        static_assert(std::same_as<xstd::bit_flag_set<std::uint8_t>, xstd::basic_bit_fixed_set<std::uint8_t, std::uint8_t, 8, xstd::bit_flag_mapping<std::uint8_t, 8>, std::greater<std::uint8_t>>>); // NOLINT(modernize-use-transparent-functors): the comparator the alias names
        static_assert(std::same_as<xstd::bit_flag_set<int>, xstd::basic_bit_fixed_set<int, std::uint32_t, 31, xstd::bit_flag_mapping<int, 31>, std::greater<int>>>);                                  // NOLINT(modernize-use-transparent-functors): the comparator the alias names
        static_assert(std::same_as<xstd::bit_flag_set<std::int8_t>, xstd::basic_bit_fixed_set<std::int8_t, std::uint8_t, 7, xstd::bit_flag_mapping<std::int8_t, 7>, std::greater<std::int8_t>>>);     // NOLINT(modernize-use-transparent-functors): the comparator the alias names
        static_assert(xstd::bit_flag_set<std::uint16_t>::max_size() == 16UZ and xstd::bit_flag_set<std::uint32_t>::max_size() == 32UZ and xstd::bit_flag_set<std::uint64_t>::max_size() == 64UZ);
        static_assert(xstd::bit_flag_set<int>::max_size() == 31UZ and xstd::bit_flag_set<std::int8_t>::max_size() == 7UZ and xstd::bit_flag_set<std::int64_t>::max_size() == 63UZ);
        static_assert(sizeof(xstd::bit_flag_set<int>) == sizeof(int) and sizeof(xstd::bit_flag_set<std::uint16_t, 12>) == sizeof(std::uint16_t) and sizeof(xstd::bit_flag_set<std::uint64_t, 5>) == 1UZ);
        static_assert(xstd::bit_mask<xstd::bit_flag_set<int>> and xstd::bit_mask<xstd::bit_flag_set<std::uint64_t>> and not xstd::bit_mask<int>);
        static_assert(std::convertible_to<int, xstd::bit_flag_set<int>> and std::convertible_to<xstd::bit_flag_set<int>, int>);
        static_assert(names_a_flag_type<std::uint8_t> and names_a_flag_type<std::int64_t> and not names_a_flag_type<bool> and not names_a_flag_type<char>);
        static_assert(names_a_flag_type<signed char> and names_a_flag_type<unsigned char> and not names_a_flag_type<char8_t> and not names_a_flag_type<char16_t> and not names_a_flag_type<wchar_t>);
        BOOST_CHECK(true);
}

// Each integer flag type converts, walks, looks up and combines with its mask on either side as the mask itself does.
BOOST_AUTO_TEST_CASE(AnIntegerMaskAgreesWithItsOwnOperators)
{
        test::for_each_type<integer_flags>([]<class X> -> void {
                BOOST_CHECK_EQUAL(integer_mismatches<X>(), 0UZ);
        });
}

// The flags run from the highest bit down, and contains takes a one-bit value: several bits, or none, are no key.
BOOST_AUTO_TEST_CASE(AnIntegerFlagTypeIteratesHighestFirstAndContainsOneBitValues)
{
        auto const wide = xstd::bit_flag_set<std::uint64_t>(0x8000'0000'0000'0101ULL);
        BOOST_CHECK(std::ranges::equal(wide, std::to_array<std::uint64_t>({0x8000'0000'0000'0000ULL, 0x100ULL, 0x1ULL})));
        auto const p = xstd::bit_flag_set<int>(0x4000'0005);
        BOOST_CHECK(std::ranges::equal(p, std::to_array({0x4000'0000, 4, 1})) and p.front() == 0x4000'0000 and p.back() == 1);
        BOOST_CHECK(p.contains(4) and p.contains(1) and not p.contains(2) and not p.contains(5) and not p.contains(0));
        BOOST_CHECK(p.find(5) == p.end() and p.count(5) == 0UZ); // NOLINT(readability-container-contains): find and count are the members under test
        BOOST_CHECK(includes(p, 5) and intersects(p, 6) and disjoint(p, 2) and not includes(p, 7));

        auto x = p;
        BOOST_CHECK(x.erase(5) == 0UZ and x == p);
        BOOST_CHECK(x.insert(2).second and x == 0x4000'0007);
        BOOST_CHECK(x.erase(2) == 1UZ and x == p);
        BOOST_CHECK_EQUAL(std::format("{}", xstd::bit_flag_set<std::int8_t>(0x51)), "{64, 16, 1}");
}

// A signed mask's sign bit is no key: & and - read a value below it, and no write brings it in.
BOOST_AUTO_TEST_CASE(ASignedFlagTypeRefusesItsSignBit)
{
        constexpr auto sign = std::numeric_limits<int>::min();
        auto p              = xstd::bit_flag_set<int>(5);
        BOOST_CHECK(int(~xstd::bit_flag_set<int>()) == std::numeric_limits<int>::max());
        BOOST_CHECK(not p.contains(sign) and not xstd::bit_flag_mapping<int>::is_key(sign));
        BOOST_CHECK_THROW(static_cast<void>(p.insert(sign)), std::out_of_range);
        BOOST_CHECK(p == 5);

        // & and - read the value below the width, and == finds a value with the sign bit unequal.
        BOOST_CHECK((p & -1) == 5 and (p - -1).empty() and (-1 & p) == 5 and p != -1 and -1 != p);
        BOOST_CHECK(std::int8_t(~xstd::bit_flag_set<std::int8_t>()) == std::numeric_limits<std::int8_t>::max());
        // A named set: Clang 23's lifetime-safety analysis crashes on an insert into a temporary here.
        auto q = xstd::bit_flag_set<std::int8_t>();
        BOOST_CHECK_THROW(static_cast<void>(q.insert(std::numeric_limits<std::int8_t>::min())), std::out_of_range);
}

// Twelve bits of sixteen, as the permissions are: & and - read the low twelve, and no write brings in a bit above.
BOOST_AUTO_TEST_CASE(ANarrowerIntegerFlagTypeRefusesTheBitsAboveIt)
{
        using narrow    = xstd::bit_flag_set<std::uint16_t, 12>;
        auto const high = std::uint16_t{0x1001};
        auto q          = narrow(std::uint16_t{0x0FFF});
        BOOST_CHECK(std::uint16_t(q & high) == 0x001U and std::uint16_t(q - high) == 0xFFEU and q != std::uint16_t{0xFFFF});
        BOOST_CHECK(not(narrow(std::uint16_t{0x001}) == high) and std::uint16_t(~narrow()) == 0x0FFFU);
        BOOST_CHECK(not q.contains(std::uint16_t{0x1000}));
        BOOST_CHECK_THROW(static_cast<void>(q.insert(std::uint16_t{0x1000})), std::out_of_range);
        BOOST_CHECK(q == std::uint16_t{0x0FFF});
}

// [ios.base]'s three bitmask types, an enumeration in one library and an integer in another, round-trip every constant.
BOOST_AUTO_TEST_CASE(EveryIosBaseConstantRoundTrips)
{
        using ios = std::ios_base;
        BOOST_CHECK_EQUAL(ios_mismatches<ios::fmtflags>({ios::boolalpha, ios::dec, ios::fixed, ios::hex, ios::internal, ios::left, ios::oct, ios::right, ios::scientific, ios::showbase, ios::showpoint, ios::showpos, ios::skipws, ios::unitbuf, ios::uppercase, ios::adjustfield, ios::basefield, ios::floatfield}), 0UZ);
        BOOST_CHECK_EQUAL(ios_mismatches<ios::iostate>({ios::goodbit, ios::badbit, ios::eofbit, ios::failbit}), 0UZ);
        BOOST_CHECK_EQUAL(ios_mismatches<ios::openmode>({ios::app, ios::ate, ios::binary, ios::in, ios::out, ios::trunc}), 0UZ);
        BOOST_CHECK(xstd::bit_flag_set<ios::iostate>(ios::goodbit).empty());
        BOOST_CHECK(xstd::bit_flag_set<ios::fmtflags>(ios::adjustfield).size() == 3UZ);
}

// Code written against a stream's flags and state, with only the variable's type respelled, formats and recovers alike.
BOOST_AUTO_TEST_CASE(AStreamTakesItsFlagsAndStateAsFlagTypes)
{
        using ios      = std::ios_base;
        using fmtflags = xstd::bit_flag_set<ios::fmtflags>;
        using iostate  = xstd::bit_flag_set<ios::iostate>;

        auto os    = std::ostringstream();
        fmtflags f = os.flags();
        f -= ios::basefield;
        f |= ios::hex;
        f |= ios::showbase;
        os.flags(f);
        os << 255;
        BOOST_CHECK_EQUAL(os.str(), "0xff");
        BOOST_CHECK(fmtflags(os.flags()) == f and f.contains(ios::hex) and not f.contains(ios::dec) and not f.contains(ios::basefield));

        auto is = std::istringstream("x");
        auto n  = 0;
        is >> n;
        iostate st = is.rdstate();
        BOOST_CHECK(st.contains(ios::failbit) and not st.contains(ios::badbit));
        st -= ios::failbit;
        is.clear(st);
        BOOST_CHECK(is.good() and iostate(is.rdstate()).empty());
}

BOOST_AUTO_TEST_SUITE_END()
