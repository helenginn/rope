#pragma once

#include "ForceScope.h"

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>
#include <vagabond/utils/ResultType.h>
#include <vagabond/utils/glm_import.h>

namespace rope::force_calc::static_scope
{
	enum class Region : std::uint8_t
	{
		Core,
		Shell,
		Outside,
	};

	// Returns one region per input position, in the same order.
	rope::utils::Result<std::vector<Region>, std::string_view>
	classifyAtomPositions(const ForceScope &force_scope,
	                      std::span<const glm::vec3> positions_angstroms);
}
