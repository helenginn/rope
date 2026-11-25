#pragma once

#include <cstdint>
#include <string_view>

namespace rope::force_calc::static_scope
{
	struct SphereError
	{
		enum class Code : std::uint8_t
		{
			NonFiniteCenter,
			NonFiniteRadius,
			NegativeRadius,
			InnerRadiusExceedsOuterRadius,
		};

		Code code;

		std::string_view toString() const
		{
			switch (code)
			{
				case Code::NonFiniteCenter:
					return "sphere center must be finite";
				case Code::NonFiniteRadius:
					return "sphere radii must be finite";
				case Code::NegativeRadius:
					return "sphere radii must be non-negative";
				case Code::InnerRadiusExceedsOuterRadius:
					return "sphere inner radius exceeds outer radius";
			}
			return "unknown sphere error";
		}
	};
}
