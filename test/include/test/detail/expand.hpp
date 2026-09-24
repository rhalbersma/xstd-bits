//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_DETAIL_EXPAND_HPP
#define TEST_DETAIL_EXPAND_HPP

#include <cstddef> // size_t
#include <tuple>   // tuple, tuple_cat
#include <utility> // declval

namespace test::detail {

// Each Block's extents, concatenated into one tuple of instantiations; declared only, for use in decltype.
template<template<class, std::size_t> class C, template<template<class, std::size_t> class, class> class Extents, class... Blocks>
auto expand(std::tuple<Blocks...>) -> decltype(std::tuple_cat(std::declval<Extents<C, Blocks>>()...));

} // namespace test::detail

#endif // TEST_DETAIL_EXPAND_HPP
