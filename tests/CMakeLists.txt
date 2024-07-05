# version 3.11 or later of CMake or needed later for installing GoogleTest
# so let's require it now.
cmake_minimum_required(VERSION 3.11-3.18)

project(tests)

add_executable(tests tests.cpp)

# We need hello.h and the hello library
target_link_libraries(tests
    PRIVATE cpputils)

# Tell C++ compiler to use C++20 features. We don't actually use any of them.
target_compile_features(tests PUBLIC cxx_std_20)