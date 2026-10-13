#ifndef __vagabond__tests_glm_printing__
#define __vagabond__tests_glm_printing__

#include <doctest/doctest.h>
#include <vagabond/utils/glm_import.h>
#include <string>

// When a CHECK compares two glm vectors, doctest prints both with operator<<
// in case the check fails. The overloads in glm_import.h are in the global
// namespace, where lookups made from inside doctest's own namespace do not
// find them, and MSVC fails to compile the check (GCC and Clang quietly
// manage). Spelling the vector out for doctest avoids operator<< altogether.
// Include this from any test which compares glm vectors.

namespace doctest
{
template <>
struct StringMaker<glm::vec3>
{
	static String convert(const glm::vec3 &v)
	{
		std::string str = "(" + std::to_string(v.x) + ", " +
		std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
		return String(str.c_str());
	}
};
}

#endif
