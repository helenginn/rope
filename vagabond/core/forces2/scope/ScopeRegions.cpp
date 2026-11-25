#include "ScopeRegions.h"

#include <cmath>
#include <utility>
#include <variant>

namespace rope::force_calc::static_scope
{
	namespace
	{
		using ClassificationResult =
		    rope::utils::Result<std::vector<Region>, std::string_view>;

		struct AtomPositionClassifier
		{
			std::span<const glm::vec3> positions_angstroms;

			ClassificationResult operator()(All) const
			{
				return rope::utils::Ok{
				    std::vector<Region>(positions_angstroms.size(), Region::Core)};
			}

			ClassificationResult operator()(const Sphere &sphere) const
			{
				std::vector<Region> regions;
				regions.reserve(positions_angstroms.size());
				for (const auto &position : positions_angstroms)
				{
					if (!std::isfinite(position.x) ||
					    !std::isfinite(position.y) ||
					    !std::isfinite(position.z))
					{
						return rope::utils::Err{"atom position must be finite"};
					}

					regions.push_back(
					    sphere.contains(position) ? Region::Core : Region::Outside);
				}
				return rope::utils::Ok{std::move(regions)};
			}

			ClassificationResult operator()(const SphereWithShell &sphere) const
			{
				const glm::dvec3 center(sphere.centerAngstroms());
				const double inner = sphere.innerRadiusAngstroms();
				const double outer = sphere.outerRadiusAngstroms();

				std::vector<Region> regions;
				regions.reserve(positions_angstroms.size());
				for (const auto &position : positions_angstroms)
				{
					const double distance =
					    glm::length(glm::dvec3(position) - center);
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
		};
	}

	rope::utils::Result<std::vector<Region>, std::string_view>
	classifyAtomPositions(const ForceScope &force_scope,
	                      std::span<const glm::vec3> positions_angstroms)
	{
		return std::visit(AtomPositionClassifier{positions_angstroms},
		                  force_scope);
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
	    glm::vec3(1.f, 2.f, 7.f),
	    center,
	    glm::vec3(1.f, 4.f, 3.f),
	    glm::vec3(4.f, 2.f, 3.f),
	};

	SUBCASE("All classifies every position as Core")
	{
		const auto result = classifyAtomPositions(All{}, positions);
		REQUIRE(result.is_ok());
		CHECK(result.unwrap() == std::vector(positions.size(), Region::Core));
	}

	SUBCASE("Sphere includes its radius boundary in the core")
	{
		const auto sphere = Sphere::create(center, 2.0);
		REQUIRE(sphere.is_ok());
		const auto result = classifyAtomPositions(sphere.unwrap(), positions);
		REQUIRE(result.is_ok());
		CHECK(result.unwrap() == std::vector{Region::Outside, Region::Core,
		                                     Region::Core, Region::Outside});
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

	SUBCASE("Spherical scopes reject a non-finite atom position")
	{
		const auto sphere = Sphere::create(center, 2.0);
		REQUIRE(sphere.is_ok());
		const auto sphere_with_shell =
		    SphereWithShell::create(center, 2.0, 3.0);
		REQUIRE(sphere_with_shell.is_ok());
		const float nan = std::numeric_limits<float>::quiet_NaN();
		const std::array invalid{glm::vec3(nan, 2.f, 3.f)};
		CHECK(classifyAtomPositions(sphere.unwrap(), invalid).is_err());
		CHECK(classifyAtomPositions(sphere_with_shell.unwrap(), invalid)
		          .is_err());
	}
}
#endif // ROPE_INLINE_TESTS
