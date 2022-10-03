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

StructureModification::StructureModification(Molecule *mol, int num, int dims)
: _sampler(num, dims)
{
	_molecule = mol;
	_num = num;
	_dims = dims;
}

StructureModification::~StructureModification()
{
	for (size_t i = 0; i < _calculators.size(); i++)
	{
		delete _calculators[i];
	}
}

void StructureModification::makeCalculator(Atom *anchor, bool has_mol)
{
	_calculators.push_back(new BondCalculator());
	BondCalculator &calc = *_calculators.back();

	calc.setPipelineType(BondCalculator::PipelineAtomPositions);
	calc.setMaxSimultaneousThreads(_threads);
	calc.setSampler(&_sampler);

	calc.setTorsionBasisType(TorsionBasis::TypeCustom);
	calc.addAnchorExtension(anchor);
	calc.setIgnoreHydrogens(false);

	customModifications(&calc, has_mol);

	calc.setup();
	calc.start();

	_num = _sampler.pointCount();
}

void StructureModification::addToHetatmCalculator(Atom *anchor)
{
	if (_hetatmCalc == nullptr)
	{
		_hetatmCalc = new BondCalculator();
		_calculators.push_back(_hetatmCalc);

		_hetatmCalc->setPipelineType(BondCalculator::PipelineAtomPositions);
		_hetatmCalc->setMaxSimultaneousThreads(1);
		_hetatmCalc->setSampler(&_sampler);

		_hetatmCalc->setIgnoreHydrogens(false);
	}

	_hetatmCalc->addAnchorExtension(anchor);
}

void StructureModification::finishHetatmCalculator()
{
	if (_hetatmCalc == nullptr)
	{
		return;
	}

	_hetatmCalc->setup();
	_hetatmCalc->start();
}

bool StructureModification::checkForMolecule(AtomGroup *grp)
{
	for (size_t i = 0; i < grp->size(); i++)
	{
		std::string ch = (*grp)[i]->chain();
		
		if (_molecule->has_chain_id(ch))
		{
			return true;
		}
	}

	return false;
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
		
		bool has_mol = checkForMolecule(_fullAtoms->connectedGroups()[i]);
		
		if (!anchor->hetatm())
		{
			makeCalculator(anchor, has_mol);
		}
		else
		{
			addToHetatmCalculator(anchor);
		}
	}
	
	finishHetatmCalculator();
}

bool StructureModification::supplyTorsions(const std::vector<ResidueTorsion> &list,
                                           const std::vector<float> &values)
{
	if (_axis >= _dims)
	{
		std::cout << "Too many axes; aborting torsion angle supply" << std::endl;
		return false;
	}

	bool success = false;
	for (BondCalculator *calc : _calculators)
	{
		TorsionBasis *basis = calc->sequenceHandler()->torsionBasis();
		ConcertedBasis *cb = static_cast<ConcertedBasis *>(basis);

		success |= fillBasis(cb, list, values, _axis);
		_torsionLists.push_back(ResidueTorsionList{list, values});
	}
	
	_axis++;
	return success;
}

bool StructureModification::fillBasis(ConcertedBasis *cb, 
                                      const std::vector<ResidueTorsion> &list,
                                      const std::vector<float> &values,
                                      int axis)
{
	bool result = cb->fillFromMoleculeList(_molecule, axis, list, values);
	checkMissingBonds(cb);
	return result;
}

void StructureModification::checkMissingBonds(ConcertedBasis *cb)
{
	for (BondTorsion *bt : cb->missingBonds())
	{
		if (bt->coversMainChain())
		{
			_mainMissing++;
		}
		else
		{
			_sideMissing++;
		}
	}
	
	if (_unusedId == nullptr)
	{
		_unusedId = cb->unusedTorsion();
	}
}

void StructureModification::changeMolecule(Molecule *m)
{
	_molecule = m;
	_sideMissing = 0;
	_mainMissing = 0;
	_unusedId = nullptr;
	_axis = 0;
	if (m != nullptr)
	{
		m->model()->load();
		_fullAtoms = m->currentAtoms();
	}
	
	bool hasCalc = _calculators.size() > 0;
	
	clearCalculators();
	
	if (hasCalc)
	{
		startCalculator();
	}

	std::vector<ResidueTorsionList> lists = _torsionLists;
	_torsionLists.clear();
	
	for (ResidueTorsionList &rtl : lists)
	{
		supplyTorsions(rtl.list, rtl.values);
	}
}

void StructureModification::clearCalculators()
{
	
	for (size_t i = 0; i < _calculators.size(); i++)
	{
		delete _calculators[i];
	}

	_calculators.clear();
}
