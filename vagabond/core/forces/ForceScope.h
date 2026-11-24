#pragma once

#include "vagabond/utils/ResultType.h"
#include <cstdint>
#include <limits>
#include <string_view>
#include "vagabond/utils/glm_import.h" // IWYU pragma: keep
// #include "Units.h" // IWYU pragma: keep //TODO: at some point, add the units back in...

namespace rope::force_calc::scope
{

	struct RadiusError
	{
		enum class Code : std::uint8_t
		{
			NonFinite,
			NegativeRadius,
			InnerExceedsOuter,
		};

		Code code;

		constexpr bool operator==(const RadiusError &) const = default;

		constexpr std::string_view toString() const
		{
			switch (code)
			{
				case Code::NonFinite:
					return "radii must be finite";
				case Code::NegativeRadius:
					return "radii must be non-negative";
				case Code::InnerExceedsOuter:
					return "inner > outer";
			}
			return "unknown radius error";
		}
	};

	class RadiusPair
	{
	public:
		static constexpr rope::utils::Result<RadiusPair, RadiusError>
		create(double inner, double outer);

		static consteval RadiusPair from_constants(double inner, double outer);
		constexpr double outer_radius() const
		{
			return _outer_radius;
		}
		constexpr double inner_radius() const
		{
			return _inner_radius;
		}

	private:
		static constexpr rope::utils::Result<void, RadiusError>
		validate_radii(double inner, double outer);

		constexpr RadiusPair(double inner, double outer)
		: _inner_radius(inner), _outer_radius(outer)
		{
		}

		double _inner_radius;
		double _outer_radius;
	};

	constexpr rope::utils::Result<void, RadiusError>

	RadiusPair::validate_radii(double inner, double outer)
	{
		// using max, because std::isfinite as constexpr is cpp23
		constexpr double max = std::numeric_limits<double>::max();

		if (!(inner >= -max && inner <= max && outer >= -max && outer <= max))
		{
			return rope::utils::Err{RadiusError{RadiusError::Code::NonFinite}};
		}

		if (inner < 0 || outer < 0)
		{
			return rope::utils::Err{
			    RadiusError{RadiusError::Code::NegativeRadius}};
		}

		if (inner > outer)
		{
			return rope::utils::Err{
			    RadiusError{RadiusError::Code::InnerExceedsOuter}};
		}

		return rope::utils::Ok();
	}

	constexpr rope::utils::Result<RadiusPair, RadiusError>
	RadiusPair::create(double inner, double outer)
	{
		auto checked_radii = validate_radii(inner, outer);

		if (checked_radii.is_err())
		{
			return rope::utils::Err{checked_radii.unwrap_err()};
		}

		return rope::utils::Ok{RadiusPair{inner, outer}};
	}

	consteval RadiusPair RadiusPair::from_constants(double inner, double outer)
	{
		return create(inner, outer).unwrap();
	}

	struct All {};

	struct Sphere
	{
    glm::vec3 center;
		RadiusPair radii;
	};

} // namespace rope::force_calc::scope
