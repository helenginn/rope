#pragma once

#include "static_balance/BalanceEquations.h"
#include "static_balance/ProxyForceSolver.h"

#include <string>
#include <vagabond/utils/ResultType.h>

class AtomGroup;

namespace rope::force_calc
{
	struct StaticForceResult
	{
		BalanceEquations equations;
		ProxyForceSolution solution;

		[[nodiscard]] Eigen::VectorXf estimates() const;
	};

	// Uses bond-angle Z-scores. Symmetry copies can be supplied in group.
	rope::utils::Result<StaticForceResult, std::string>
	analyzeStaticForces(const AtomGroup &group);
}
