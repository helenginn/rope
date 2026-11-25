#include "ScopeSelection.h"

#include "../geometry/ForceTopology.h"

#include <utility>
#include <variant>

namespace rope::force_calc::static_scope
{
	rope::utils::Result<ScopeSelection, std::string_view>
	selectSystemForScope(const ForceScope &force_scope,
	                     std::span<const glm::vec3> positions_angstroms,
	                     const ForceTopology &topology,
	                     std::span<const std::size_t> proxy_candidates)
	{
		if (positions_angstroms.size() != topology.atom_count)
		{
			return rope::utils::Err{"positions must match the topology"};
		}

		auto regions = classifyAtomPositions(force_scope, positions_angstroms);
		if (regions.is_err())
		{
			return rope::utils::Err{regions.unwrap_err()};
		}

		ScopeSelection selection{std::move(regions).unwrap(), {}, {}, {}};
		std::vector<bool> kept_bonds(topology.bonds.size());
		std::vector<bool> has_kept_bond(topology.atom_count);

		for (std::size_t i = 0; i < topology.bonds.size(); ++i)
		{
			const auto [left, right] = topology.bonds[i];
			if (selection.atom_regions[left] == Region::Outside ||
			    selection.atom_regions[right] == Region::Outside)
			{
				continue;
			}

			selection.bond_ids.push_back(i);
			kept_bonds[i] = true;
			has_kept_bond[left] = true;
			has_kept_bond[right] = true;
		}

		for (std::size_t i = 0; i < topology.angles.size(); ++i)
		{
			const auto [left, right] = topology.angles[i];
			if (kept_bonds[left] && kept_bonds[right])
			{
				selection.angle_ids.push_back(i);
			}
		}

		const bool all = std::holds_alternative<All>(force_scope);
		for (const auto atom : proxy_candidates)
		{
			if (atom >= topology.atom_count)
			{
				return rope::utils::Err{"invalid proxy candidate"};
			}
			if (all || has_kept_bond[atom])
			{
				selection.proxy_atoms.push_back(atom);
			}
		}

		return rope::utils::Ok{std::move(selection)};
	}
}

#ifdef ROPE_INLINE_TESTS
#include <array>
#include <doctest/doctest.h>

TEST_CASE("selectSystemForScope keeps original IDs across the outer cutoff")
{
	using rope::force_calc::ForceTopology;
	using namespace rope::force_calc::static_scope;

	const ForceTopology topology{
	    5,
	    {{0, 1}, {2, 3}, {1, 2}},
	    {{2, 1}, {0, 2}},
	};
	const std::array positions{
	    glm::vec3(0.f),
	    glm::vec3(2.f, 0.f, 0.f),
	    glm::vec3(3.f, 0.f, 0.f),
	    glm::vec3(4.f, 0.f, 0.f),
	    glm::vec3(0.5f, 0.f, 0.f),
	};
	const std::array<std::size_t, 5> candidates{0, 1, 2, 3, 4};

	SUBCASE("All keeps every bond, angle, and supplied proxy candidate")
	{
		const auto result =
		    selectSystemForScope(All{}, positions, topology, candidates);
		REQUIRE(result.is_ok());
		CHECK(result.unwrap().bond_ids == std::vector<std::size_t>{0, 1, 2});
		CHECK(result.unwrap().angle_ids == std::vector<std::size_t>{0, 1});
		CHECK(result.unwrap().proxy_atoms ==
		      std::vector<std::size_t>{0, 1, 2, 3, 4});
	}

	SUBCASE("Only the outer radius affects the selected equations and unknowns")
	{
		const auto sphere = Sphere::create(glm::vec3(0.f), 2.0, 3.0);
		REQUIRE(sphere.is_ok());
		const auto result = selectSystemForScope(sphere.unwrap(), positions,
		                                         topology, candidates);
		REQUIRE(result.is_ok());
		const auto &selection = result.unwrap();
		CHECK(selection.atom_regions[1] == Region::Core);
		CHECK(selection.atom_regions[2] == Region::Shell);
		CHECK(selection.bond_ids == std::vector<std::size_t>{0, 2});
		CHECK(selection.angle_ids == std::vector<std::size_t>{1});
		CHECK(selection.proxy_atoms == std::vector<std::size_t>{0, 1, 2});

		const auto smaller_core = Sphere::create(glm::vec3(0.f), 1.0, 3.0);
		REQUIRE(smaller_core.is_ok());
		const auto smaller = selectSystemForScope(
		    smaller_core.unwrap(), positions, topology, candidates);
		REQUIRE(smaller.is_ok());
		CHECK(smaller.unwrap().atom_regions[1] == Region::Shell);
		CHECK(smaller.unwrap().bond_ids == selection.bond_ids);
		CHECK(smaller.unwrap().angle_ids == selection.angle_ids);
		CHECK(smaller.unwrap().proxy_atoms == selection.proxy_atoms);
	}
}
#endif // ROPE_INLINE_TESTS
