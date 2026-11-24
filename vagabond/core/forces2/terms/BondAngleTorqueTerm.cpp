#include "BondAngleTorqueTerm.h"

#include "../geometry/BondAngleGeometry.h"
#include "../geometry/ForceTopology.h"
#include "../models/BondAngleTorque.h"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/BondAngle.h>

namespace rope::force_calc
{
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
