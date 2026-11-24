#pragma once

#include <string_view>
#include <vagabond/utils/Eigen/Core>
#include <vagabond/utils/ResultType.h>

namespace rope::force_calc
{
	struct BalanceEquations;

	struct ProxyForceSolution
	{
		Eigen::VectorXf
		    components; // XYZ in BalanceEquations::proxy_atoms order
		Eigen::Index rank;
	};

	rope::utils::Result<ProxyForceSolution, std::string_view>
	solveProxyForces(const BalanceEquations &equations);
}
