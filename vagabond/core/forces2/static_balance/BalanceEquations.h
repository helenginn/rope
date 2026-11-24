#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>
#include <vagabond/utils/Eigen/SparseCore>
#include <vagabond/utils/ResultType.h>
#include <vagabond/utils/glm_import.h>

namespace rope::force_calc
{
	struct ForceTopology;

	struct BalanceInput
	{
		std::span<const glm::vec3> initial_positions_angstroms; // Per atom
		std::span<const glm::vec3> applied_forces;              // Per atom
		std::span<const glm::vec3> applied_torques;             // Per bond
		std::span<const std::size_t> proxy_atoms; // Atoms to be solved
	};

	struct BalanceEquations
	{
		BalanceEquations() = default;

		// Eigen allocates an empty sparse matrix before swap.
		// NOLINTNEXTLINE(performance-noexcept-move-constructor)
		BalanceEquations(BalanceEquations &&other);

		Eigen::SparseMatrix<float> matrix;
		Eigen::VectorXf target;
		std::vector<std::size_t>
		    proxy_atoms; // Three columns per atom, in this order
	};

	// Builds A * x = b using z-scores
	// Each bond contributes three force rows followed by three moment rows
	rope::utils::Result<BalanceEquations, std::string_view>
	buildBalanceEquations(const ForceTopology &topology,
	                      const BalanceInput &input);
}
