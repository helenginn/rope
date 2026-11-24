#include "BondAngleGeometry.h"
#include "glm/geometric.hpp"

#include <cmath>
#include <vagabond/core/Atom.h>
#include <vagabond/core/BondAngle.h>

namespace rope::force_calc
{
	std::string_view BondAngleGeometryError::toString() const
	{
		switch (code)
		{
			case Code::NonFiniteGeometry:
				return "bond-angle geometry is not finite";
			case Code::InvalidStandardDeviation:
				return "bond-angle standard deviation must be finite and "
				       "positive";
			case Code::DegenerateAxis:
				return "initial positions do not define a bond-angle axis";
		}
		return "unknown bond-angle geometry error";
	}

	rope::utils::Result<BondAngleGeometry, BondAngleGeometryError>
	readBondAngleGeometry(const BondAngle &angle)
	{
		using Error = BondAngleGeometryError;
		using Code = Error::Code;

		const double stdev = angle.stdev();
		if (!std::isfinite(stdev) || stdev <= 0.0)
		{
			return rope::utils::Err{Error{Code::InvalidStandardDeviation}};
		}
		const double measured = angle.measurement();
		const double expected = angle.expectation();
		if (!std::isfinite(measured) || !std::isfinite(expected))
		{
			return rope::utils::Err{Error{Code::NonFiniteGeometry}};
		}

		const auto &a = angle.atom(0)->initialPosition();
		const auto &b = angle.atom(1)->initialPosition();
		const auto &c = angle.atom(2)->initialPosition();

		const glm::vec3 normal = glm::cross(c - a, b - a);
		const float length_squared = glm::dot(normal, normal);
		if (!std::isfinite(length_squared))
		{
			return rope::utils::Err{Error{Code::NonFiniteGeometry}};
		}
		if (length_squared == 0.f)
		{
			return rope::utils::Err{Error{Code::DegenerateAxis}};
		}

		return rope::utils::Ok{BondAngleGeometry{
		    au::degrees(measured),
		    au::degrees(expected),
		    au::degrees(stdev),
		    glm::normalize(normal),
		}};
	}
} // namespace rope::force_calc

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>

#include <limits>

TEST_CASE("readBondAngleGeometry reads and validates bond-angle geometry")
{
	using rope::force_calc::readBondAngleGeometry;
	using Code = rope::force_calc::BondAngleGeometryError::Code;

	Atom left, centre, right;
	left.setInitialPosition({2.f, 0.f, 0.f});
	centre.setInitialPosition({0.f, 0.f, 0.f});
	right.setInitialPosition({0.f, 3.f, 0.f});
	right.setDerivedPosition({1.f, 0.f, 1.f});
	const BondAngle angle(nullptr, &left, &centre, &right, 100.0, 2.0);

	SUBCASE("measures the derived angle and uses the initial axis")
	{
		auto result = readBondAngleGeometry(angle);
		REQUIRE(result.is_ok());
		const auto &geometry = result.unwrap();
		CHECK(geometry.measured.in(au::degrees) == doctest::Approx(45.0));
		CHECK(geometry.expected.in(au::degrees) == doctest::Approx(100.0));
		CHECK(geometry.standard_deviation.in(au::degrees) ==
		      doctest::Approx(2.0));
		CHECK(geometry.initial_axis.x == doctest::Approx(0.f));
		CHECK(geometry.initial_axis.y == doctest::Approx(0.f));
		CHECK(geometry.initial_axis.z == doctest::Approx(1.f));
	}

	SUBCASE("rejects zero standard deviation")
	{
		const BondAngle invalid(nullptr, &left, &centre, &right, 100.0, 0.0);
		auto result = readBondAngleGeometry(invalid);
		REQUIRE(result.is_err());
		CHECK(result.unwrap_err().code == Code::InvalidStandardDeviation);
	}

	SUBCASE("rejects non-finite geometry")
	{
		const double nan = std::numeric_limits<double>::quiet_NaN();
		const BondAngle invalid(nullptr, &left, &centre, &right, nan, 2.0);
		auto result = readBondAngleGeometry(invalid);
		REQUIRE(result.is_err());
		CHECK(result.unwrap_err().code == Code::NonFiniteGeometry);
	}

	SUBCASE("rejects collinear initial positions")
	{
		right.setInitialPosition({-1.f, 0.f, 0.f});
		auto result = readBondAngleGeometry(angle);
		REQUIRE(result.is_err());
		CHECK(result.unwrap_err().code == Code::DegenerateAxis);
	}
}

#endif // ROPE_INLINE_TESTS
