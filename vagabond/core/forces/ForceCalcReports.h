#pragma once

#include <string>
#include <vector>
#include <optional>
#include <vagabond/utils/Eigen/Sparse>

// TODO: still missing / to cover print statements:
//
// weights for the matrix
// residuals, relative errors?
// degrees of freedom, i.e. quations + unknowns
// diagnosis data: charge, symmetry, runtime, etc?

namespace rope::force_calc
{

	struct StressStrainRow
	{
		std::string desc;
		float stress;
		float strain;
	};

	struct TargetEstimateRow
	{
		float target;
		float estimate;
	};

	struct TorsionElectromagneticRow
	{
		float normalized_dot_product;
		std::string desc;
	};

	struct ForceCalcResultReport
	{
		std::optional<float> stress_strain_correlation;

		std::vector<StressStrainRow> stress_strain;
		std::vector<TargetEstimateRow> target_estimate;
		std::vector<TorsionElectromagneticRow> torsion_electromagnetic;
		Eigen::SparseMatrix<float, Eigen::ColMajor> weights;

		static ForceCalcResultReport create()
		{
			return ForceCalcResultReport{
			    std::nullopt,
			    {},
			    {},
			    {},
          {},
			};
		}
	};
} // namespace rope::force_calc
