#include "BondAngleTorque.h"

#include "../geometry/ForceTopology.h"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/BondAngle.h>

namespace rope::force_calc
{
	LegacyBondAngleTorque
	legacyBondAngleTorque(const BondAngleGeometry &geometry)
	{
		// BondAngle::as_signed_z_score takes the measurement as a float.
		const float observed =
		    static_cast<float>(geometry.observed.in(au::degrees));
		const double ideal = geometry.ideal.in(au::degrees);
		const double stdev = geometry.standard_deviation.in(au::degrees);
		const float z_score = static_cast<float>((observed - ideal) / stdev);

		return {geometry.initial_axis, -z_score};
	}

	rope::utils::Result<void, std::string>
	addBondAngleTorques(const AtomGroup &group, const ForceTopology &topology,
	                    std::span<glm::vec3> torques)
	{
		if (torques.size() != topology.bonds.size() ||
		    topology.angles.size() != group.bondAngleCount() ||
		    !std::in_range<int>(topology.angles.size()))
		{
			return rope::utils::Err{"bond-angle inputs do not match the topology"};
		}

		for (std::size_t i = 0; i < topology.angles.size(); ++i)
		{
			const auto &angle = *group.bondAngle(static_cast<int>(i));
			if (angle.atom(0)->elementSymbol() == "H" ||
			    angle.atom(2)->elementSymbol() == "H")
			{
				continue;
			}

			const auto geometry = readBondAngleGeometry(angle);
			if (geometry.is_err())
			{
				return rope::utils::Err{
				    "bond angle " + std::to_string(i) + ": " +
				    std::string(geometry.unwrap_err().toString())};
			}

			const auto torque = legacyBondAngleTorque(geometry.unwrap());
			if (!std::isfinite(torque.signed_magnitude))
			{
				return rope::utils::Err{
				    "non-finite torque for bond angle " + std::to_string(i)};
			}

			const glm::vec3 contribution = torque.axis * torque.signed_magnitude;
			const auto [left_bond, right_bond] = topology.angles[i];
			torques[left_bond] += contribution;
			torques[right_bond] -= contribution;
		}

		return rope::utils::Ok{};
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

	geometry.observed = au::degrees(105.0);
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
