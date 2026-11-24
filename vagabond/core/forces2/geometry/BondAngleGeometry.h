#pragma once

#include <au/units/degrees.hh>
#include <cstdint>
#include <string_view>
#include <vagabond/utils/ResultType.h>
#include <vagabond/utils/glm_import.h>

class BondAngle;

namespace rope::force_calc
{
	using AngleDegrees = au::Quantity<au::Degrees, double>;

	struct BondAngleGeometry
	{
		AngleDegrees observed; // From derived atom positions.
		AngleDegrees ideal;
		AngleDegrees standard_deviation;
		glm::vec3 initial_axis; // Normalised vec from inital position
	};

	struct BondAngleGeometryError
	{
		enum class Code : std::
		    uint8_t // uint8 for explicitly smaller enum class
		{
			NonFiniteGeometry,
			InvalidStandardDeviation,
			DegenerateAxis,
		};
		Code code;
		std::string_view toString() const;
	};

	rope::utils::Result<BondAngleGeometry, BondAngleGeometryError>
	readBondAngleGeometry(const BondAngle &angle);

}
