#include "ScopeRegions.h"

#include <cmath>
#include <utility>
#include <variant>

namespace rope::force_calc::static_scope
{
	rope::utils::Result<std::vector<Region>, std::string_view>
	classifyAtomPositions(const ForceScope &force_scope,
	                      std::span<const glm::vec3> positions_angstroms)
	{
		if (std::holds_alternative<All>(force_scope))
		{
			return rope::utils::Ok{
			    std::vector<Region>(positions_angstroms.size(), Region::Core)};
		}

		const auto &sphere = std::get<SphereWithShell>(force_scope);
		const glm::dvec3 center(sphere.centerAngstroms());
		const double inner = sphere.innerRadiusAngstroms();
		const double outer = sphere.outerRadiusAngstroms();

		std::vector<Region> regions;
		regions.reserve(positions_angstroms.size());
		for (const auto &position : positions_angstroms)
		{
			const double distance = glm::length(glm::dvec3(position) - center);
			if (!std::isfinite(distance))
			{
				return rope::utils::Err{"atom position must be finite"};
			}

			if (distance <= inner)
			{
				regions.push_back(Region::Core);
			}
			else if (distance <= outer)
			{
				regions.push_back(Region::Shell);
			}
			else
			{
				regions.push_back(Region::Outside);
			}
		}

		return rope::utils::Ok{std::move(regions)};
	}
}

#ifdef ROPE_INLINE_TESTS
#include <array>
#include <doctest/doctest.h>
#include <limits>

TEST_CASE("classifyAtomPositions preserves order and includes both boundaries")
{
	using namespace rope::force_calc::static_scope;

	const glm::vec3 center(1.f, 2.f, 3.f);
	const std::array positions{
	    glm::vec3(1.f, 2.f, 7.f), center,
	    glm::vec3(1.f, 4.f, 3.f), glm::vec3(4.f, 2.f, 3.f),
	};

	SUBCASE("All classifies every position as Core")
	{
		const auto result = classifyAtomPositions(All{}, positions);
		REQUIRE(result.is_ok());
		CHECK(result.unwrap() == std::vector(positions.size(), Region::Core));
	}

	SUBCASE("SphereWithShell includes atoms at both radii")
	{
		const auto sphere = SphereWithShell::create(center, 2.0, 3.0);
		REQUIRE(sphere.is_ok());
		const auto result = classifyAtomPositions(sphere.unwrap(), positions);
		REQUIRE(result.is_ok());
		CHECK(result.unwrap() == std::vector{Region::Outside, Region::Core,
		                                    Region::Core, Region::Shell});
	}

	SUBCASE("SphereWithShell rejects a non-finite atom position")
	{
		const auto sphere = SphereWithShell::create(center, 2.0, 3.0);
		REQUIRE(sphere.is_ok());
		const float nan = std::numeric_limits<float>::quiet_NaN();
		const std::array invalid{glm::vec3(nan, 2.f, 3.f)};
		CHECK(classifyAtomPositions(sphere.unwrap(), invalid).is_err());
	}
}
#endif // ROPE_INLINE_TESTS
