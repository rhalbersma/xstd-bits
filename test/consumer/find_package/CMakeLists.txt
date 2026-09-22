#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.28)
project(consumer_find_package LANGUAGES CXX)

# Three install trees: this library's own, xstd's (find_dependency'd) and vcpkg's, for the Boost the headers use.
find_package(xstd-bits 0.1.0 CONFIG REQUIRED)

add_executable(main ../main.cpp)
target_link_libraries(main PRIVATE xstd::bits)
