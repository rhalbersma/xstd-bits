//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_INPUT_HPP
#define TEST_SPEC_INPUT_HPP

#include <boost/test/results_collector.hpp> // results_collector
#include <boost/test/unit_test.hpp>         // BOOST_ERROR, counter_t, current_test_case_id
#include <array>                            // array
#include <concepts>                         // integral
#include <cstddef>                          // ptrdiff_t, size_t
#include <cstdint>                          // uint64_t
#include <initializer_list>                 // initializer_list
#include <map>                              // map
#include <ostream>                          // ostream
#include <ranges>                           // iota
#include <tuple>                            // apply, tuple
#include <utility>                          // declval, move
#include <vector>                           // vector

// The operands a clause is checked over, each carrying where it came from, so a failure names its input.
namespace test::spec {

// A named edge case or exhaustive enumeration at its sweep width, or a sample its seed and key counts reproduce.
struct origin
{
        char const* tier = "edge";
        char const* name = "";
        std::size_t width = 0;
        std::uint64_t seed = 0;
        std::size_t sample = 0;
        std::array<std::size_t, 3> keys = {};
        std::size_t operands = 0;

        friend auto operator<<(std::ostream& os, origin const& from)
                -> std::ostream&
        {
                if (from.operands == 0) {
                        return os << from.tier << ": " << from.name << ", width " << from.width;
                }
                os << from.tier << ": seed " << from.seed << ", width " << from.width << ", keys " << from.keys[0];
                if (from.operands == 3) {
                        os << ", " << from.keys[1];
                }
                if (from.operands >= 2) {
                        os << " and " << from.keys[from.operands - 1];
                }
                return os << ", sample " << from.sample;
        }
};

[[nodiscard]] constexpr auto edge(char const* name, std::size_t width) noexcept
        -> origin
{
        return {.tier = "edge", .name = name, .width = width};
}

[[nodiscard]] constexpr auto exhaustive(char const* name, std::size_t width) noexcept
        -> origin
{
        return {.tier = "exhaustive", .name = name, .width = width};
}

// Each input is its origin followed by its operands, which a case takes apart with a structured binding.
#ifdef __clang__

// An operand is of any size, so an input pads wherever its type puts it.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"

#endif

template<class X>
struct one
{
        origin from;
        X a;
};

template<class X>
struct two
{
        origin from;
        X a;
        X b;
};

template<class X>
struct three
{
        origin from;
        X a;
        X b;
        X c;
};

// An object and a key or a bit position, which the clause may or may not require to be valid.
template<class X>
struct keyed
{
        origin from;
        X a;
        std::size_t k;
};

// An object and two keys or positions: two indices, or where a range starts and how long it is.
template<class X>
struct keyed_pair
{
        origin from;
        X a;
        std::size_t k;
        std::size_t l;
};

// A set and keys in the order a caller hands them over.
template<class X>
struct listed
{
        origin from;
        X a;
        std::vector<std::size_t> keys;
};

#ifdef __clang__

#pragma clang diagnostic pop

#endif

using key_vector = std::vector<std::size_t>;

using key_list = one<key_vector>;

// An input rebuilt over another type, its operands made by make and the rest copied.
template<class X, class K>
constexpr auto rebuild(one<K> const& x, auto make)
        -> one<X>
{
        return {.from = x.from, .a = make(x.a)};
}

template<class X, class K>
constexpr auto rebuild(two<K> const& x, auto make)
        -> two<X>
{
        return {.from = x.from, .a = make(x.a), .b = make(x.b)};
}

template<class X, class K>
constexpr auto rebuild(three<K> const& x, auto make)
        -> three<X>
{
        return {.from = x.from, .a = make(x.a), .b = make(x.b), .c = make(x.c)};
}

template<class X, class K>
constexpr auto rebuild(keyed<K> const& x, auto make)
        -> keyed<X>
{
        return {.from = x.from, .a = make(x.a), .k = x.k};
}

template<class X, class K>
constexpr auto rebuild(keyed_pair<K> const& x, auto make)
        -> keyed_pair<X>
{
        return {.from = x.from, .a = make(x.a), .k = x.k, .l = x.l};
}

template<class X, class K>
constexpr auto rebuild(listed<K> const& x, auto make)
        -> listed<X>
{
        return {.from = x.from, .a = make(x.a), .keys = x.keys};
}

// Inputs enumerated over a carrier of keys or positions, each rebuilt over X only as a case reads it.
template<class X, class Carrier, class Make>
class rebuilt
{
        std::vector<std::vector<Carrier> const*> m_shared;
        std::vector<std::vector<Carrier>> m_owned;

        [[nodiscard]] constexpr auto segments() const noexcept
                -> std::size_t
        {
                return m_shared.size() + m_owned.size();
        }

        [[nodiscard]] constexpr auto segment(std::size_t s) const noexcept
                -> std::vector<Carrier> const&
        {
                return s < m_shared.size() ? *m_shared[s] : m_owned[s - m_shared.size()];
        }

public:
        using value_type = decltype(rebuild<X>(std::declval<Carrier const&>(), Make()));

        // A position as the segment it is in and the carrier in that segment, the end being one past the last segment.
        class iterator
        {
                rebuilt const* m_inputs = nullptr;
                std::size_t m_segment = 0;
                std::size_t m_carrier = 0;

                constexpr auto skip_empty() noexcept
                        -> void
                {
                        while (m_segment < m_inputs->segments() and m_carrier == m_inputs->segment(m_segment).size()) {
                                ++m_segment;
                                m_carrier = 0;
                        }
                }

        public:
                using value_type = rebuilt::value_type;
                using difference_type = std::ptrdiff_t;

                [[nodiscard]] constexpr iterator() = default;

                [[nodiscard]] constexpr iterator(rebuilt const* inputs, std::size_t s) noexcept
                        : m_inputs(inputs)
                        , m_segment(s)
                {
                        skip_empty();
                }

                [[nodiscard]] constexpr auto operator*() const
                        -> value_type
                {
                        return rebuild<X>(m_inputs->segment(m_segment)[m_carrier], Make());
                }

                constexpr auto operator++() noexcept
                        -> iterator&
                {
                        ++m_carrier;
                        skip_empty();
                        return *this;
                }

                constexpr auto operator++(int) noexcept
                        -> iterator
                {
                        auto const old = *this;
                        ++*this;
                        return old;
                }

                [[nodiscard]] friend constexpr auto operator==(iterator const&, iterator const&) noexcept -> bool = default;
        };

        [[nodiscard]] constexpr rebuilt() = default;

        [[nodiscard]] constexpr auto begin() const noexcept
                -> iterator
        {
                return iterator(this, 0);
        }

        [[nodiscard]] constexpr auto end() const noexcept
                -> iterator
        {
                return iterator(this, segments());
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> std::size_t
        {
                auto n = 0UZ;
                for (auto const s : std::views::iota(0UZ, segments())) {
                        n += segment(s).size();
                }
                return n;
        }

        // Another enumeration's inputs after these, held here, as a constant expression needs them.
        constexpr auto append(std::vector<Carrier> carriers)
                -> void
        {
                m_owned.push_back(std::move(carriers));
        }

        // Another enumeration's inputs after these, held by the cache they were taken from.
        auto share(std::vector<Carrier> const& carriers)
                -> void
        {
                m_shared.push_back(&carriers);
        }
};

// An enumeration's carriers for these arguments, computed once and shared by every case and type that asks for them.
template<auto Enumerate, class... Args>
[[nodiscard]] auto memo(Args... args)
        -> decltype(Enumerate(args...)) const&
{
        // Never destroyed, so no exit-time destructor runs.
        static auto& cache = *new std::map<std::tuple<Args...>, decltype(Enumerate(args...))>();
        auto const [it, inserted] = cache.try_emplace(std::tuple(args...));
        if (inserted) {
                it->second = Enumerate(args...);
        }
        return it->second;
}

// A bitset streams itself; a set or a list of keys is spelled as its keys in braces.
template<class X>
auto print(std::ostream& os, X const& x)
        -> void
{
        if constexpr (std::integral<X> or requires { os << x; }) {
                os << x;
        } else {
                os << '{';
                auto const* separator = "";
                for (auto const k : x) {
                        os << separator << static_cast<std::size_t>(k);
                        separator = ", ";
                }
                os << '}';
        }
}

// The origin and operands of an input, reported only if a check on it failed, at the one place a context type decides.
class failure_report
{
public:
        using printer = auto (*)(std::ostream&, void const*) -> void;

        [[nodiscard]] failure_report(origin const& from, void const* operands, printer print)
                : m_from(from)
                , m_operands(operands)
                , m_print(print)
                , m_failed(failed())
        {}

        auto report() const
                -> void
        {
                if (failed() != m_failed) {
                        BOOST_ERROR("the failure above is on input " << *this);
                }
        }

        friend auto operator<<(std::ostream& os, failure_report const& x)
                -> std::ostream&
        {
                os << x.m_from << ";";
                x.m_print(os, x.m_operands);
                return os;
        }

private:
        origin const& m_from;
        void const* m_operands;
        printer m_print;
        boost::unit_test::counter_t m_failed;

        [[nodiscard]] static auto failed()
                -> boost::unit_test::counter_t
        {
                return boost::unit_test::results_collector.results(boost::unit_test::framework::current_test_case_id()).p_assertions_failed;
        }
};

// The checks on one input, for the length of a scope, after which a failure among them names the input.
template<class... Operands>
class context
{
        std::tuple<Operands const&...> m_operands;
        failure_report m_report;

        static auto print_all(std::ostream& os, void const* operands)
                -> void
        {
                std::apply([&](auto const&... operand) -> void { ((os << ' ', print(os, operand)), ...); }, *static_cast<std::tuple<Operands const&...> const*>(operands));
        }

public:
        [[nodiscard]] explicit context(origin const& from, Operands const&... operands)
                : m_operands(operands...)
                , m_report(from, &m_operands, &print_all)
        {}

        context(context const&) = delete;
        context(context&&) = delete;
        auto operator=(context const&) -> context& = delete;
        auto operator=(context&&) -> context& = delete;

        ~context()
        {
                m_report.report();
        }
};

// Only a braced list builds an initializer list, so the keys are handed over as one where there are at most two.
auto with_initializer_list(std::vector<std::size_t> const& keys, auto fun)
        -> void
{
        if (keys.empty()) {
                fun(std::initializer_list<std::size_t>{});
        } else if (keys.size() == 1) {
                fun(std::initializer_list<std::size_t>{keys[0]});
        } else if (keys.size() == 2) {
                fun(std::initializer_list<std::size_t>{keys[0], keys[1]});
        }
}

// A mutating primitive applied to a copy, so the input is there unchanged for the next check.
auto on_copy(auto fun, auto const& a, auto const&... args)
        -> void
{
        auto x = a;
        fun(x, args...);
}

} // namespace test::spec

#endif // TEST_SPEC_INPUT_HPP
