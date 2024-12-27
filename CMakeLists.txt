cmake_minimum_required(VERSION 3.20)
project(DPDKPP
  DESCRIPTION "C++ library for DPDK"
  LANGUAGES CXX C
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED IMPORTED_TARGET libdpdk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_compile_options(-g)
add_compile_options(-Wall -Wextra -Werror)

add_subdirectory(examples)
