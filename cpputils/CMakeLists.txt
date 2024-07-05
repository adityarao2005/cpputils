# version 3.11 or later of CMake or needed later for installing GoogleTest
# so let's require it now.
cmake_minimum_required(VERSION 3.11-3.18)

project(cpputils)

set(BUILD_SHARED_LIBS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS True)
set(CMAKE_CXX_STANDARD 20)

add_library(cpputils src/cpputils.cpp src/debug.cpp src/chrono.cpp src/format.cpp)

# PUBLIC needed to make both hello.h and hello library available elsewhere in project
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Tell compiler to use C++20 features. The code doesn't actually use any of them.
target_compile_features(cpputils PUBLIC cxx_std_20)

# Copy the cpputils.dll to the output directory
add_custom_command(TARGET cpputils POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:cpputils>"
     "../apps/$<CONFIGURATION>/$<TARGET_FILE_NAME:cpputils>" 
  COMMENT "Copying to output directory")