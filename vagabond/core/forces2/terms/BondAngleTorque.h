#pragma once

#include "../geometry/BondAngleGeometry.h"

#include <span>
#include <string>
#include <vagabond/utils/ResultType.h>

class AtomGroup;

namespace rope::force_calc
{
	struct ForceTopology;

	struct LegacyBondAngleTorque
	{
		glm::vec3 axis;
		float signed_magnitude;
	};

	[[nodiscard]] LegacyBondAngleTorque
	legacyBondAngleTorque(const BondAngleGeometry &geometry);

	// Use the topology read from group and one slot per bond.
	// On error, discard the partially accumulated torques.
	rope::utils::Result<void, std::string>
	addBondAngleTorques(const AtomGroup &group, const ForceTopology &topology,
	                    std::span<glm::vec3> torques);
}
