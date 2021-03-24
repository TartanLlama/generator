# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("generator")

add_library(generator INTERFACE)
target_include_directories(generator INTERFACE generator)

add_executable(generator_test tests/test.cpp )

if(MSVC)
target_compile_options(generator_test PRIVATE /std:c++latest)
else()
target_compile_options(generator_test PRIVATE -std=c++20 -fcoroutines -fconcepts-diagnostics-depth=5)
endif()
target_link_libraries(generator_test PRIVATE generator)

