// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "VagabondPositions.h"
#include "PositionRefinery.h"
#include "AtomGroup.h"
#include "AlignmentTool.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "TorsionBasis.h"
#include "SimplexEngine.h"
#include "ChemotaxisEngine.h"
#include "Result.h"

PositionRefinery::PositionRefinery(AtomGroup *group)
{
	_group = group;
	_group->recalculate();
}

PositionRefinery::~PositionRefinery()
{

}

void PositionRefinery::backgroundRefine(PositionRefinery *ref)
{
	ref->refine();
}

void PositionRefinery::refineThroughEach(AtomGroup *subset)
{
	VagabondPositions vagabond(subset);
	vagabond.setup();

	float before = vagabond.fullResidual();

	int depth = _thorough ? 10 : 5;
	size_t nb = vagabond.totalPositions();

	for (size_t i = 0; i < nb; i++)
	{
		bool improved = vagabond.refineBetween(i, i + 8);

		if (_finish)
		{
			break;
		}
	}

	vagabond.updateAllTorsions();
	float after = vagabond.fullResidual();

	std::cout << "Overall average distance after refinement: "
	<< before << " before to " << after << " Angstroms over " 
	<< subset->size() << " atoms." << std::endl;
}

void PositionRefinery::refine()
{
	if (_group == nullptr)
	{
		throw std::runtime_error("Attempting to refine group, but no "
		                         "group specified.");
	}
	
	std::vector<AtomGroup *> subsets = _group->connectedGroups(false);

	for (AtomGroup *subset : subsets)
	{
		if (subset->size() <= 1)
		{
			continue;
		}

		try
		{
			refineThroughEach(subset);
		}
		catch (const std::runtime_error &err)
		{
			std::cout << "Giving up: " << err.what() << std::endl;
		}
	}
	
	_group->finishedRefining();
	
	_done = true;
}


void PositionRefinery::finish()
{
	_finish = true;
}


