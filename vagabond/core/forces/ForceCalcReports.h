#pragma once

#include <string>
#include <vector>
#include <optional>

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

		static ForceCalcResultReport create()
		{
			return ForceCalcResultReport{
			    std::nullopt,
			    {},
			    {},
			    {},
			};
		}
	};
} // namespace rope::force_calc
