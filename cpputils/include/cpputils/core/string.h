#pragma once

#include <cstddef>
#include <cpputils/cpputils_api.h>
#include <string>

namespace cpputils
{
	using cstring = char *;
	using icstring = const char *;
	using string = std::string;
	using cwstring = wchar_t *;
	using icwstring = const wchar_t *;
	using wstring = std::wstring;
}