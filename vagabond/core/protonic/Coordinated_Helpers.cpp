// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#define HYDROGEN_BONDING_TOLERANCE (45.0f)
#define HYDROGEN_MAX_DISTANCE (3.5f)
#define PLANAR_TOLERANCE (45.0f)

#include <iostream>

#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/glm_import.h>
#include "matrix_functions.h"
#include "CovalentProbe.h"
#include "Coordinated.h"
#include "Covalent2Count.h"
#include "OrCount.h"
#include "CountProbe.h"
#include "BondAngle.h"
#include "AtomGroup.h"
#include "Energy.h"
#include "Guilt.h"
#include <gemmi/elem.hpp>

using namespace hnet;

#include "Coordinated_Helpers.h"

namespace hnet
{

bool is_placeholder_hydrogen_name(const std::string &name)
{
	return name.rfind("H!", 0) == 0;
}

OpSet<ABPair> turn_into_pairset(const CovPairSet &set)
{
	OpSet<ABPair> ret;
	for (auto &element : set)
	{
		ret += {element.first, nullptr};
	}
	return ret;
};

bool are_equivalent(const PairSet &a_all, const PairSet &b_all)
{
	auto strip_fake_h = [](const ABPair &pair)
	{
		return !is_placeholder_hydrogen_name(pair.first.ptr->atomName());
	};

	PairSet a = a_all.filter(strip_fake_h);
	PairSet b = b_all.filter(strip_fake_h);

	if (a.size() != b.size())
	{
		return false;
	}

	auto a_it = a.begin(); auto b_it = b.begin();

	while (a_it != a.end() && b_it != b.end())
	{
		if (a_it->second != b_it->second)
		{
			return false;
		}

		a_it++; b_it++;
	}

	return true;
};

bool are_equivalent(const AcceptableGroup &a_all, const AcceptableGroup &b_all)
{
	if (a_all.coord_num != b_all.coord_num)
	{
		return false;
	}

	return are_equivalent(a_all.group, b_all.group);
};

void add_unique_to_set(OpSet<AcceptableGroup> &dest,
                       const AcceptableGroup &newest)
{
	bool found = false;
	for (const AcceptableGroup &old_solution : dest)
	{
		if (are_equivalent(old_solution, newest))
		{
			found = true;
		}

	}

	if (found)
	{
		std::cout << "Skipping equivalent group, already seen." << std::endl;
		return;
	}

	dest += newest;
}

OpSet<PairSet> convert_pair_set_to_all_relationships(const PairSet &start)
{
	OpSet<PairSet> relationships;

	for (const ABPair &left : start)
	{
		for (const ABPair &right : start)
		{
			if (left == right) continue;

			PairSet pair_up;
			pair_up += left; pair_up += right;
			relationships.insert(pair_up);
		}
	}

	return relationships;
}

}
