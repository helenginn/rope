#pragma once

#include "ForceScope.h"
#include "ScopeRegions.h"

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>
#include <vagabond/utils/ResultType.h>
#include <vagabond/utils/glm_import.h>

namespace rope::force_calc
{
	struct ForceTopology;
}

namespace rope::force_calc::static_scope
{
	struct ScopeSelection
	{
		std::vector<Region> atom_regions; // Indexed by original atom ID
		std::vector<std::size_t> bond_ids;
		std::vector<std::size_t> angle_ids;
		std::vector<std::size_t> proxy_atoms;
	};

	// Use the topology from readForceTopology and unique proxy candidate IDs.
	rope::utils::Result<ScopeSelection, std::string_view>
	selectSystemForScope(const ForceScope &force_scope,
	                     std::span<const glm::vec3> positions_angstroms,
	                     const ForceTopology &topology,
	                     std::span<const std::size_t> proxy_candidates);
}
