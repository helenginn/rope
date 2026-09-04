#pragma once

#include "vagabond/core/TabulatedData.h"

struct ForceCalcResultReport
{
	float stress_strain_correlation;

	TabulatedData stress_strain;
	TabulatedData target_estimates;
	TabulatedData torsion_electromagnetic;
};
