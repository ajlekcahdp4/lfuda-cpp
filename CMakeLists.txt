cmake_minimum_required (VERSION 3.23)

project (lfudacpp)
set (LFUDA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set (BELADY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_program(BASH_PROGRAM bash)
enable_testing()

add_subdirectory(test)
