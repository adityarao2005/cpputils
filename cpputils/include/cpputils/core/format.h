#pragma once

#include <cpputils/cpputils_api.h>
#include <cpputils/core/string.h>
#include <sstream>
#include <cpputils/core/collections.h>

namespace cpputils
{
	// Format function
	template <typename... Args>
	string cformat(const string &fmt, Args... args)
	{
		// Calculate the size of the buffer
		size_t size = snprintf(nullptr, 0, fmt.c_str(), args...) + 1;

		// Create the buffer
		char buffer[size];

		// Format the string
		snprintf(buffer, size, fmt.c_str(), args...);

		// Return the formatted string
		return buffer;
	}

	// To string function
	// This function is used to convert a value to a string
	template <typename T>
	string to_string(const T &value);

	// Specializations
	template <>
	inline string to_string(const cstring &value)
	{
		return value;
	}

	template <>
	inline string to_string(const icstring &value)
	{
		return value;
	}

	template <>
	inline string to_string(const int &value)
	{
		return cformat("%d", value);
	}

	template <>
	inline string to_string(const unsigned int &value)
	{
		return cformat("%u", value);
	}

	template <>
	inline string to_string(const long &value)
	{
		return cformat("%ld", value);
	}

	template <>
	inline string to_string(const unsigned long &value)
	{
		return cformat("%lu", value);
	}

	template <>
	inline string to_string(const long long &value)
	{
		return cformat("%lld", value);
	}

	template <>
	inline string to_string(const unsigned long long &value)
	{
		return cformat("%llu", value);
	}

	template <>
	inline string to_string(const float &value)
	{
		return cformat("%f", value);
	}

	template <>
	inline string to_string(const double &value)
	{
		return cformat("%f", value);
	}

	template <>
	inline string to_string(const std::string &value)
	{
		return value;
	}

	// Format function for strings
	// Basically the internal working of the below function "format"
	// Handles how the strings will be placed into the format
	CPPUTILS_API string format_str(const string &fmt, const array_list<string> &values);

	// Format function
	// This function is used to format a string using a format string and a list of arguments
	// The string could look like this: "Hello, {}! You are \{{}\} years old."
	// and arguments could be: "World", 25
	// The result would be: "Hello, World! You are {25} years old."
	// We would neglect any other parameters passed but if there are less params then we would throw error
	template <typename... Args>
	string format(const string &fmt, Args... args)
	{
		// Convert it all to strings and then pass to format_str
		return format_str(fmt, {to_string(args)...});
	}

}