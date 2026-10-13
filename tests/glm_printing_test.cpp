#include <doctest/doctest.h>
#include "glm_printing.h"

TEST_CASE("glm vectors can be compared in a CHECK and printed by doctest")
{
	glm::vec3 a(1.f, 2.5f, -3.f);

	CHECK(a == glm::vec3(1.f, 2.5f, -3.f));
	CHECK(a != glm::vec3(1.f, 2.5f, 3.f));

	// what doctest shows when such a check fails
	CHECK(doctest::toString(a) == "(1.000000, 2.500000, -3.000000)");
}
