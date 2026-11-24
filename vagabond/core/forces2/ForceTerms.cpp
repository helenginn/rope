#include "ForceTerms.h"

namespace rope::force_calc
{
	LegacyBondAngleTorque
	legacyBondAngleTorque(const BondAngleGeometry &geometry)
	{
		// BondAngle::as_signed_z_score takes the measurement as a float.
		const float measured =
		    static_cast<float>(geometry.measured.in(au::degrees));
		const double expected = geometry.expected.in(au::degrees);
		const double stdev = geometry.standard_deviation.in(au::degrees);
		const float z_score = static_cast<float>((measured - expected) / stdev);

		return {geometry.initial_axis, -z_score};
	}
}

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>

TEST_CASE("legacyBondAngleTorque preserves the axis and signed magnitude")
{
	using namespace rope::force_calc;

	BondAngleGeometry geometry{
	    au::degrees(45.0),
	    au::degrees(100.0),
	    au::degrees(2.0),
	    {0.f, 0.f, 1.f},
	};

	const auto torque = legacyBondAngleTorque(geometry);
	CHECK(torque.axis == geometry.initial_axis);
	CHECK(torque.signed_magnitude == 27.5f);

	geometry.measured = au::degrees(105.0);
	CHECK(legacyBondAngleTorque(geometry).signed_magnitude == -2.5f);
}

TEST_CASE("legacyBondAngleTorque rounds the measurement before subtraction")
{
	using namespace rope::force_calc;

	const BondAngleGeometry geometry{
	    au::degrees(90.000001),
	    au::degrees(90.0),
	    au::degrees(2.0),
	    {0.f, 0.f, 1.f},
	};

	CHECK(legacyBondAngleTorque(geometry).signed_magnitude == 0.f);
}
#endif // ROPE_INLINE_TESTS
