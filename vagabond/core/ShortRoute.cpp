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

#include "ShortRoute.h"
#include "BondCalculator.h"
#include "Grapher.h"
#include "BondSequence.h"

ShortRoute::ShortRoute(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims)
: Route(mol, cluster, dims)
{

}

bool ShortRoute::prepareSegment(Atom *atom)
{
	setup();
	submitJob(0);

	int keep = -1;
	AnchorExtension ext{};

	for (size_t i = 0; i < _calculators.size(); i++)
	{
		const BondSequence *seq = _calculators[i]->sequence();
		ext = seq->getExtension(atom);

		if (ext.parent != nullptr)
		{
			keep = i;
			break; // stop looking
		}

		// this calculator didn't actually find this atom
	}

	if (keep < 0)
	{
		return false;
	}
	
	BondCalculator *chosen = _calculators[keep];
	_calculators.erase(_calculators.begin() + keep);
	/* we will repurpose chosen, and get rid of the others */
	clearCalculators();
	
	chosen->reset();
	chosen->addAnchorExtension(ext);
	chosen->setup();
	chosen->start();
	
	return true;
}
