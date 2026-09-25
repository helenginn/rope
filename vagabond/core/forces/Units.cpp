#include "Units.h"

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>

using namespace rope::force_calc::units;

TEST_CASE("angstrom converts to meters")
{
	const auto length = angstroms(1.0);

	CHECK(length.in(au::meters) == doctest::Approx(1e-10));
}

TEST_CASE("meters convert to angstroms")
{
	const auto length = au::meters(1e-10);

	CHECK(length.in(angstroms) == doctest::Approx(1.0));
}

TEST_CASE("angstrom conversion preserves scale")
{
	const auto length = angstroms(5.0);

	CHECK(length.in(au::meters) == doctest::Approx(5e-10));
}

#endif
