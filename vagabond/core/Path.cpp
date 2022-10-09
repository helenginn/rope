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

#include "Path.h"
#include "ModelManager.h"

Path::Path(PlausibleRoute *pr)
{
	_route = pr;
	_startMolecule = pr->molecule()->id();
	_endMolecule = pr->endMolecule()->id();
	_model_id = pr->molecule()->model()->id();
	_molecule = pr->molecule();
	_end = pr->endMolecule();

	_wayPoints = pr->_wayPoints;
	_flips = pr->_flips;
	_type = pr->_type;
	_destination = pr->_destination;
	
	for (size_t i = 0; i < pr->_torsions.size(); i++)
	{
		getTorsionRef(i);
	}
	
	_route = nullptr;
}

void Path::getTorsionRef(int idx)
{
	BondTorsion *t = _route->_torsions[idx];

	ResidueId id = t->residueId();
	TorsionRef rt = TorsionRef(t);
	rt.setRefinedAngle(_destination[idx]);

	_residueIds.push_back(id);
	_torsionRefs.push_back(rt);
}

void Path::housekeeping()
{
	_destination.clear();
	for (TorsionRef &ref : _torsionRefs)
	{
		_destination.push_back(ref.refinedAngle());
	}

}

PlausibleRoute *Path::toRoute()
{
	if (_route != nullptr)
	{
		return _route;
	}

	_model = Environment::env().modelManager()->model(_model_id);
	
	for (Molecule &m : _model->molecules())
	{
		if (m.id() == _startMolecule)
		{
			_molecule = &m;
		}
		
	}
	
	for (Model &m : Environment::env().modelManager()->objects())
	{
		for (Molecule &mol : m.molecules())
		{
			if (mol.id() == _endMolecule)
			{
				_end = &mol;
			}
		}
	}
	
	if (!_molecule)
	{
		throw std::runtime_error("Unable to find molecule in environment.");
	}
	
	_model->load();
	AtomGroup *group = _model->currentAtoms();

	PlausibleRoute *pr = new PlausibleRoute(_molecule, nullptr, 
	                                        _wayPoints.size());
	pr->_destination = _destination;
	pr->_type = _type;
	pr->_flips = _flips;
	pr->_wayPoints = _wayPoints;
	pr->_endMolecule = _end;

	for (size_t i = 0; i < _residueIds.size(); i++)
	{
		Residue *local = _molecule->sequence()->residueLike(_residueIds[i]);
		BondTorsion *bt = nullptr;
		
		if (local)
		{
			for (size_t j = 0; j < group->bondTorsionCount(); j++)
			{
				BondTorsion *c = group->bondTorsion(j);
				
				const std::string &ch = c->atom(1)->chain();
				if (!_molecule->has_chain_id(ch))
				{
					continue;
				}
				
				if (c->residueId().str() != local->id().str())
				{
					continue;
				}
				
				if (c->desc() != _torsionRefs[i].desc() &&
				    c->reverse_desc() != _torsionRefs[i].desc())
				{
					continue;
				}
				
				bt = c;
			}
		}
		
		if (bt == nullptr)
		{
			std::cout << "WARNING! null bond " << local->id().str() << std::endl;
		}
		
		pr->_torsions.push_back(bt);
	}

	pr->clearMask();
	_route = pr;
	
	return pr;
}

std::string Path::desc() const
{
	return _startMolecule + " to " + _endMolecule;
}

void Path::calculateArrays(MetadataGroup *group)
{
	const int total = 32;

	_molecule->model()->load();
	AtomContent *grp = _molecule->model()->currentAtoms();
	PlausibleRoute *pr = toRoute();
	pr->setup();
	pr->calculateProgression(total);
	
	for (size_t i = 0; i < total; i++)
	{
		pr->submitJobAndRetrieve(i);
		_molecule->extractTorsionAngles(grp, true);
		MetadataGroup::Array vals = _molecule->grabTorsions(true);
		group->matchDegrees(vals);
		_angleArrays.push_back(vals);
	}
	
	_molecule->model()->unload();

}

void Path::addTorsionsToGroup(MetadataGroup &group)
{
	if (_angleArrays.size() == 0)
	{
		calculateArrays(&group);
	}
	
	if (_contributeSVD)
	{
		for (size_t i = 0; i < _angleArrays.size(); i++)
		{
			group.addMetadataArray(this, _angleArrays[i]);
		}
	}

}
