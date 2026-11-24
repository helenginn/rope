#pragma once

#include <span>
#include <string>
#include <vagabond/utils/ResultType.h>
#include <vagabond/utils/glm_import.h>

class AtomGroup;

namespace rope::force_calc
{
	struct ForceTopology;

	// Use the topology read from group and one slot per bond.
	// On error, discard the partially accumulated torques.
	rope::utils::Result<void, std::string>
	addBondAngleTorques(const AtomGroup &group, const ForceTopology &topology,
	                    std::span<glm::vec3> torques);
}
