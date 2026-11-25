#include "Sphere.h"

#include <cmath>

namespace rope::force_calc::static_scope
{
	rope::utils::Result<Sphere, SphereError>
	Sphere::create(glm::vec3 center_angstroms, double radius_angstroms)
	{
		using Code = SphereError::Code;

		if (!std::isfinite(center_angstroms.x) ||
		    !std::isfinite(center_angstroms.y) ||
		    !std::isfinite(center_angstroms.z))
		{
			return rope::utils::Err{SphereError{Code::NonFiniteCenter}};
		}
		if (!std::isfinite(radius_angstroms))
		{
			return rope::utils::Err{SphereError{Code::NonFiniteRadius}};
		}
		if (radius_angstroms < 0.0)
		{
			return rope::utils::Err{SphereError{Code::NegativeRadius}};
		}

		return rope::utils::Ok{Sphere{center_angstroms, radius_angstroms}};
	}

	bool Sphere::contains(const glm::vec3 &position_angstroms) const
	{
		const glm::dvec3 delta =
		    glm::dvec3(position_angstroms) - glm::dvec3(_center_angstroms);
		return glm::length(delta) <= _radius_angstroms;
	}
}

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>
#include <limits>

TEST_CASE("Sphere::create validates its center and radius")
{
	using rope::force_calc::static_scope::Sphere;
	using rope::force_calc::static_scope::SphereError;
	using Code = SphereError::Code;

	const glm::vec3 center(1.f, 2.f, 3.f);
	const double infinity = std::numeric_limits<double>::infinity();
	const float nan = std::numeric_limits<float>::quiet_NaN();

	const auto valid = Sphere::create(center, 2.0);
	REQUIRE(valid.is_ok());
	CHECK(valid.unwrap().centerAngstroms() == center);
	CHECK(valid.unwrap().radiusAngstroms() == 2.0);

	const auto bad_center = Sphere::create({0.f, 0.f, nan}, 2.0);
	REQUIRE(bad_center.is_err());
	CHECK(bad_center.unwrap_err().code == Code::NonFiniteCenter);

	const auto bad_radius = Sphere::create(center, infinity);
	REQUIRE(bad_radius.is_err());
	CHECK(bad_radius.unwrap_err().code == Code::NonFiniteRadius);

	const auto negative = Sphere::create(center, -1.0);
	REQUIRE(negative.is_err());
	CHECK(negative.unwrap_err().code == Code::NegativeRadius);
}

TEST_CASE("Sphere::contains includes points on the boundary")
{
	using rope::force_calc::static_scope::Sphere;

	const glm::vec3 center(1.f, 2.f, 3.f);
	const auto result = Sphere::create(center, 2.0);
	REQUIRE(result.is_ok());
	CHECK(result.unwrap().contains(center));
	CHECK(result.unwrap().contains({1.f, 4.f, 3.f}));
	CHECK_FALSE(result.unwrap().contains({1.f, 4.f, 4.f}));

	const auto point = Sphere::create(center, 0.0);
	REQUIRE(point.is_ok());
	CHECK(point.unwrap().contains(center));
	CHECK_FALSE(point.unwrap().contains({1.f, 3.f, 3.f}));
}
#endif // ROPE_INLINE_TESTS
