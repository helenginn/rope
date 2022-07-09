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

#include "StructureModification.h"
#include <vagabond/core/BondSequenceHandler.h>
#include <vagabond/core/Molecule.h>
#include <vagabond/core/ConcertedBasis.h>

StructureModification::StructureModification(Molecule *mol)
: _sampler(1, 1)
{
	_molecule = mol;
}

StructureModification::~StructureModification()
{

}

void StructureModification::makeCalculator(Atom *anchor)
{
	_calculators.push_back(new BondCalculator());
	BondCalculator &calc = *_calculators.back();

	calc.setPipelineType(BondCalculator::PipelineAtomPositions);
	calc.setMaxSimultaneousThreads(1);
	calc.setSampler(&_sampler);

	calc.setTorsionBasisType(TorsionBasis::TypeCustom);

	calc.addAnchorExtension(anchor);

	calc.setIgnoreHydrogens(false);
	calc.setup();

	calc.start();

	_num = _sampler.pointCount();

	TorsionBasis *basis = calc.sequenceHandler()->torsionBasis();
	ConcertedBasis *cb = static_cast<ConcertedBasis *>(basis);
	
	fillBasis(cb);
}

void StructureModification::startCalculator()
{
	for (size_t i = 0; i < _fullAtoms->connectedGroups().size(); i++)
	{
		Atom *anchor = _fullAtoms->connectedGroups()[i]->chosenAnchor();
		
		if (anchor->hetatm())
		{
			continue;
		}
		
		if (_molecule->has_chain_id(anchor->chain()))
		{
			makeCalculator(anchor);
		}
	}
}

