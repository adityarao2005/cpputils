#pragma once

#include <cpputils/cpputils_api.h>
#include <chrono>
#include <cpputils/format.h>
#include <sstream>
#include <iomanip>

namespace cpputils
{
	// chrono namespace
	namespace chrono
	{
		// Define the time_point type
		using time_point = std::chrono::time_point<std::chrono::system_clock>;

		// Define the duration type
		using duration = std::chrono::duration<double>;

		// Define the time units
		using year = std::chrono::year;

		using month = std::chrono::month;

		using day = std::chrono::day;

		using hours = std::chrono::hours;

		using minutes = std::chrono::minutes;

		using seconds = std::chrono::seconds;

		using milliseconds = std::chrono::milliseconds;

		using microseconds = std::chrono::microseconds;

		using nanoseconds = std::chrono::nanoseconds;

		// Get the current time point
		CPPUTILS_API chrono::time_point now();

		// Get the time difference in seconds
		CPPUTILS_API double diff(const chrono::time_point &start, const chrono::time_point &end);
	}
	// Define the duration literals
	using namespace std::chrono_literals;

	// converts to string
	template <>
	inline string to_string(const chrono::time_point &value)
	{
		auto tt = std::chrono::system_clock::to_time_t(value);
		std::tm tm = *std::gmtime(&tt); // GMT (UTC)
		// std::tm tm = *std::localtime(&tt); // Local time
		std::stringstream ss;
		ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
		return ss.str();
	}
}