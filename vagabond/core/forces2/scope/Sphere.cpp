#include "Sphere.h"

#include <cmath>

namespace rope::force_calc::static_scope
{
	std::string_view ScopeError::toString() const
	{
		switch (code)
		{
			case Code::NonFiniteCenter:
				return "sphere center must be finite";
			case Code::NonFiniteRadius:
				return "sphere radii must be finite";
			case Code::NegativeRadius:
				return "sphere radii must be non-negative";
			case Code::InnerRadiusExceedsOuterRadius:
				return "sphere inner radius exceeds outer radius";
		}
		return "unknown scope error";
	}

	rope::utils::Result<Sphere, ScopeError>
	Sphere::create(glm::vec3 center_angstroms, double inner_radius_angstroms,
	               double outer_radius_angstroms)
	{
		using Code = ScopeError::Code;

		if (!std::isfinite(center_angstroms.x) ||
		    !std::isfinite(center_angstroms.y) ||
		    !std::isfinite(center_angstroms.z))
		{
			return rope::utils::Err{ScopeError{Code::NonFiniteCenter}};
		}
		if (!std::isfinite(inner_radius_angstroms) ||
		    !std::isfinite(outer_radius_angstroms))
		{
			return rope::utils::Err{ScopeError{Code::NonFiniteRadius}};
		}
		if (inner_radius_angstroms < 0.0 || outer_radius_angstroms < 0.0)
		{
			return rope::utils::Err{ScopeError{Code::NegativeRadius}};
		}
		if (inner_radius_angstroms > outer_radius_angstroms)
		{
			return rope::utils::Err{
			    ScopeError{Code::InnerRadiusExceedsOuterRadius}};
		}

		return rope::utils::Ok{Sphere{center_angstroms, inner_radius_angstroms,
		                              outer_radius_angstroms}};
	}
}

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>
#include <limits>

TEST_CASE("Sphere::create validates its center and radii")
{
	using rope::force_calc::static_scope::ScopeError;
	using rope::force_calc::static_scope::Sphere;
	using Code = ScopeError::Code;

	const glm::vec3 center(1.f, 2.f, 3.f);
	const double infinity = std::numeric_limits<double>::infinity();
	const float nan = std::numeric_limits<float>::quiet_NaN();

	const auto valid = Sphere::create(center, 2.0, 5.0);
	REQUIRE(valid.is_ok());
	CHECK(valid.unwrap().centerAngstroms() == center);
	CHECK(valid.unwrap().innerRadiusAngstroms() == 2.0);
	CHECK(valid.unwrap().outerRadiusAngstroms() == 5.0);
	CHECK(Sphere::create(center, 0.0, 0.0).is_ok());
	CHECK(Sphere::create(center, 2.0, 2.0).is_ok());

	const auto bad_center = Sphere::create({nan, 0.f, 0.f}, 2.0, 5.0);
	REQUIRE(bad_center.is_err());
	CHECK(bad_center.unwrap_err().code == Code::NonFiniteCenter);

	const auto bad_radius = Sphere::create(center, 2.0, infinity);
	REQUIRE(bad_radius.is_err());
	CHECK(bad_radius.unwrap_err().code == Code::NonFiniteRadius);

	const auto negative = Sphere::create(center, -1.0, 5.0);
	REQUIRE(negative.is_err());
	CHECK(negative.unwrap_err().code == Code::NegativeRadius);

	const auto reversed = Sphere::create(center, 5.0, 2.0);
	REQUIRE(reversed.is_err());
	CHECK(reversed.unwrap_err().code == Code::InnerRadiusExceedsOuterRadius);
}
#endif // ROPE_INLINE_TESTS
