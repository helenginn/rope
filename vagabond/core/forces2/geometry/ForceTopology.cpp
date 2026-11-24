#include "ForceTopology.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <utility>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/BondAngle.h>
#include <vagabond/core/BondLength.h>

namespace rope::force_calc
{
	namespace
	{
		using AtomIds = std::unordered_map<const Atom *, std::size_t>;
		using BondKey = std::pair<std::size_t, std::size_t>;
		using BondIds = std::map<BondKey, std::size_t>;
		using Error = ForceTopologyError;
		using Code = Error::Code;

		struct IndexedBonds
		{
			std::vector<BondEndpoints> endpoints;
			BondIds ids;
		};

		BondKey bondKey(std::size_t a, std::size_t b)
		{
			return std::minmax(a, b);
		}

		AtomIds indexAtoms(const AtomVector &atoms)
		{
			AtomIds ids;
			ids.reserve(atoms.size());
			for (std::size_t i = 0; i < atoms.size(); ++i)
			{
				ids.emplace(atoms[i], i);
			}
			return ids;
		}

		rope::utils::Result<IndexedBonds, Error>
		readBonds(const AtomGroup &group, const AtomIds &atom_ids)
		{
			IndexedBonds bonds;
			bonds.endpoints.reserve(group.bondLengthCount());

			for (std::size_t i = 0; i < group.bondLengthCount(); ++i)
			{
				auto *bond = group.bondLength(i);
				const auto left = atom_ids.find(bond->atom(0));
				const auto right = atom_ids.find(bond->atom(1));
				if (left == atom_ids.end() || right == atom_ids.end() ||
				    left == right)
				{
					return rope::utils::Err{Error{Code::InvalidBondEndpoints, i}};
				}

				const auto key = bondKey(left->second, right->second);
				if (!bonds.ids.emplace(key, i).second)
				{
					return rope::utils::Err{Error{Code::DuplicateBond, i}};
				}
				bonds.endpoints.push_back({left->second, right->second});
			}

			return rope::utils::Ok{std::move(bonds)};
		}

		rope::utils::Result<std::vector<AngleBonds>, Error>
		readAngleBonds(const AtomGroup &group, const AtomIds &atom_ids,
		               const BondIds &bond_ids)
		{
			std::vector<AngleBonds> angles;
			angles.reserve(group.bondAngleCount());

			for (std::size_t i = 0; i < group.bondAngleCount(); ++i)
			{
				const auto &angle = *group.bondAngle(i);
				const auto a = atom_ids.find(angle.atom(0));
				const auto b = atom_ids.find(angle.atom(1));
				const auto c = atom_ids.find(angle.atom(2));
				if (a == atom_ids.end() || b == atom_ids.end() ||
				    c == atom_ids.end())
				{
					return rope::utils::Err{Error{Code::InvalidAngleBonds, i}};
				}

				const auto left = bond_ids.find(bondKey(a->second, b->second));
				const auto right = bond_ids.find(bondKey(b->second, c->second));
				if (left == bond_ids.end() || right == bond_ids.end() ||
				    left == right)
				{
					return rope::utils::Err{Error{Code::InvalidAngleBonds, i}};
				}
				angles.push_back({left->second, right->second});
			}

			return rope::utils::Ok{std::move(angles)};
		}
	}

	std::string ForceTopologyError::toString() const
	{
		switch (code)
		{
			case Code::InvalidBondEndpoints:
				return "invalid endpoints for bond " + std::to_string(index);
			case Code::DuplicateBond:
				return "duplicate endpoints for bond " + std::to_string(index);
			case Code::InvalidAngleBonds:
				return "angle " + std::to_string(index) +
				       " must connect two distinct bonds in the structure";
		}
		return "unknown topology error";
	}

	rope::utils::Result<ForceTopology, ForceTopologyError>
	readForceTopology(const AtomGroup &group)
	{
		const auto &atoms = group.atomVector();
		const auto atom_ids = indexAtoms(atoms);

		auto bonds = readBonds(group, atom_ids);
		if (bonds.is_err())
		{
			return rope::utils::Err{bonds.unwrap_err()};
		}

		auto &indexed_bonds = bonds.unwrap();
		auto angles = readAngleBonds(group, atom_ids, indexed_bonds.ids);
		if (angles.is_err())
		{
			return rope::utils::Err{angles.unwrap_err()};
		}

		return rope::utils::Ok{ForceTopology{
		    atoms.size(),
		    std::move(indexed_bonds.endpoints),
		    std::move(angles).unwrap(),
		}};
	}
}

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>
#include <vagabond/core/Atom.h>

TEST_CASE("readForceTopology maps angles to their two bonds")
{
	using namespace rope::force_calc;

	Atom a("GLY", "N");
	Atom b("GLY", "CA");
	Atom c("GLY", "C");
	AtomGroup group;
	group.add(&c);
	group.add(&a);
	group.add(&b);
	BondLength right(&group, &c, &b, 1.0);
	BondAngle angle(&group, &a, &b, &c, 100.0, 2.0);

	SUBCASE("keeps input indices and matches bonds in either orientation")
	{
		BondLength left(&group, &b, &a, 1.0);
		auto result = readForceTopology(group);
		REQUIRE(result.is_ok());
		const auto &topology = result.unwrap();
		CHECK(topology.atom_count == 3);
		REQUIRE(topology.bonds.size() == 2);
		CHECK(topology.bonds[0].left_atom == 0);
		CHECK(topology.bonds[0].right_atom == 2);
		CHECK(topology.bonds[1].left_atom == 2);
		CHECK(topology.bonds[1].right_atom == 1);
		REQUIRE(topology.angles.size() == 1);
		CHECK(topology.angles[0].left_bond == 1);
		CHECK(topology.angles[0].right_bond == 0);
	}

	SUBCASE("reports an angle with a missing bond")
	{
		auto result = readForceTopology(group);
		REQUIRE(result.is_err());
		CHECK(result.unwrap_err().code ==
		      ForceTopologyError::Code::InvalidAngleBonds);
		CHECK(result.unwrap_err().index == 0);
	}
}
#endif // ROPE_INLINE_TESTS
