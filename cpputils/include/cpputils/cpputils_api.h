#pragma once

// Define the API export
#ifdef _WIN32
#define CPPUTILS_API __declspec(dllexport)
#else
#define CPPUTILS_API __attribute__((visibility("default")))
#endif

// Define the version
#define CPPUTILS_VERSION_MAJOR 1
#define CPPUTILS_VERSION_MINOR 0
#define CPPUTILS_VERSION_PATCH 0

// Define the version code
#define CPPUTILS_VERSION(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))

// Define the version code
#define CPPUTILS_VERSION_CODE CPPUTILS_VERSION(CPPUTILS_VERSION_MAJOR, CPPUTILS_VERSION_MINOR, CPPUTILS_VERSION_PATCH)

// #define EXPERIMENTAL