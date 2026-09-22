//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_TAGS_HPP
#define XSTD_BITS_DETAIL_TAGS_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

namespace xstd {

// The readings, flat: a bitset is a hybrid of the other two rather than a refinement, so no tag nests inside another.
struct bitset_reading_tag
{};

struct sequence_reading_tag
{};

struct set_reading_tag
{};

// Opt-in rather than closed, the way std::ranges::enable_view is, so a reading declared elsewhere can join the set.
template<class T>
inline constexpr bool enable_reading_tag = false;

template<>
inline constexpr bool enable_reading_tag<bitset_reading_tag> = true;

template<>
inline constexpr bool enable_reading_tag<sequence_reading_tag> = true;

template<>
inline constexpr bool enable_reading_tag<set_reading_tag> = true;

template<class T>
concept reading_tag = enable_reading_tag<T>;

// What holds the blocks: a static extent, a run-time width over a static capacity, and an allocating one.
struct array_container_tag
{};

struct vector_container_tag
{};

template<class T>
inline constexpr bool enable_container_tag = false;

template<>
inline constexpr bool enable_container_tag<array_container_tag> = true;

template<>
inline constexpr bool enable_container_tag<vector_container_tag> = true;

// Absent where the standard library has no std::inplace_vector, so the concept never holds for a tag with no storage.
#ifdef __cpp_lib_inplace_vector

struct inplace_vector_container_tag
{};

template<>
inline constexpr bool enable_container_tag<inplace_vector_container_tag> = true;

#endif // __cpp_lib_inplace_vector

template<class T>
concept container_tag = enable_container_tag<T>;

} // namespace xstd

#endif // XSTD_BITS_DETAIL_TAGS_HPP
