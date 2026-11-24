#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <vagabond/utils/ResultType.h>

class AtomGroup;

namespace rope::force_calc
{
	struct BondEndpoints
	{
		std::size_t left_atom;
		std::size_t right_atom;
	};

	struct AngleBonds
	{
		std::size_t left_bond;
		std::size_t right_bond;
	};

	// Indices according to atomVector(), bondLength(), and bondAngle() in input
	struct ForceTopology
	{
		std::size_t atom_count;
		std::vector<BondEndpoints> bonds;
		std::vector<AngleBonds> angles;
	};

	struct ForceTopologyError
	{
		enum class Code : std::uint8_t // limit enum size
		{
			InvalidBondEndpoints,
			DuplicateBond,
			InvalidAngleBonds,
		};

		Code code;
		std::size_t index; // Index of the offending bond or angle in the input
		std::string toString() const;
	};

	rope::utils::Result<ForceTopology, ForceTopologyError>
	readForceTopology(const AtomGroup &group);
}
