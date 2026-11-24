#pragma once

#include "../geometry/BondAngleGeometry.h"

namespace rope::force_calc
{
	struct LegacyBondAngleTorque
	{
		glm::vec3 axis;
		float signed_magnitude;
	};

	[[nodiscard]] LegacyBondAngleTorque
	legacyBondAngleTorque(const BondAngleGeometry &geometry);
}
