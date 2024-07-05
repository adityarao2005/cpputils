#pragma once

#include <cpputils/cpputils_api.h>
#include <memory>

namespace cpputils
{
	template <typename T>
	using ref = std::shared_ptr<T>;

	template <typename T>
	using uref = std::unique_ptr<T>;

	// Make a shared pointer
	template <typename T, typename... Args>
	ref<T> make_ref(Args... args)
	{
		return std::make_shared<T>(args...);
	}

	// Make a unique pointer
	template <typename T, typename... Args>
	uref<T> make_uref(Args... args)
	{
		return std::make_unique<T>(args...);
	}
}