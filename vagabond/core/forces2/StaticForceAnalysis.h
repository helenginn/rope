#pragma once

#include "scope/ForceScope.h"
#include "static_balance/BalanceEquations.h"
#include "static_balance/ProxyForceSolver.h"

#include <cstddef>
#include <string>
#include <vector>
#include <vagabond/utils/ResultType.h>
#include <vagabond/utils/glm_import.h>

class AtomGroup;

namespace rope::force_calc
{
	struct ProxyForce
	{
		std::size_t atom_id;
		glm::vec3 force;
	};

	struct StaticForceResult
	{
		BalanceEquations equations;
		ProxyForceSolution solution;

		[[nodiscard]] Eigen::VectorXf estimates() const;
		[[nodiscard]] std::vector<ProxyForce> coreProxyForces() const;
	};

	// Uses bond-angle Z-scores. Symmetry copies can be supplied in group.
	// Currently supports the All scope.
	rope::utils::Result<StaticForceResult, std::string>
	analyzeStaticForces(const AtomGroup &group,
	                    const static_scope::ForceScope &force_scope =
	                        static_scope::All{});
}
