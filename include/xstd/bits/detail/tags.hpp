//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_TAGS_HPP
#define XSTD_BITS_DETAIL_TAGS_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

namespace xstd {

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
