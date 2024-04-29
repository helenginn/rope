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

#include "Ligand.h"
#include "Environment.h"
#include "ModelManager.h"
#include "AtomGroup.h"
#include "RTAngles.h"
#include "Model.h"

Ligand::Ligand(std::string model_id, AtomGroup *grp)
{
	_model_id = model_id;
	_currentAtoms = nullptr;
	_anchorDesc = grp->chosenAnchor()->desc();
	_code = grp->chosenAnchor()->code();
	_chain = grp->chosenAnchor()->chain();
	
	for (const Atom *a : grp->atomVector())
	{
		const ResidueId &id = a->residueId();
		_resids.insert(id);
	}
}

Ligand::Ligand()
{
	setRefined(true); // false but needed for now
}

std::string Ligand::desc() const
{
	std::string d = _model_id + ":" + _chain + "/" + _code;
	
	if (_resids.size())
	{
		d += _resids.begin()->str();
	}
	
	return d;
}

bool Ligand::atomBelongsToInstance(Atom *a)
{
	if (_chain != a->chain())
	{
		return false;
	}

	return (_resids.count(a->residueId()) > 0);
}


const std::string Ligand::id() const
{
	return _model_id + ":" + _anchorDesc;
}

Atom *Ligand::equivalentForAtom(Ligand *other, Atom *atom)
{
	load();
	AtomGroup *mine = currentAtoms();

	for (Atom *candidate : mine->atomVector())
	{
		if (candidate->atomName() == atom->atomName())
		{
			return candidate;
		}
	}
	
	unload();
	return nullptr;
}

void Ligand::collectTorsions()
{
	if (_rts.size() > 0)
	{
		return;
	}

	load();
	_rts.clear();
	AtomGroup *mine = currentAtoms();
	
	for (size_t i = 0; i < mine->bondTorsionCount(); i++)
	{
		BondTorsion *t = mine->bondTorsion(i);
		ResidueTorsion rt(t);
		_rts.push_back(ResidueTorsion(t));
	}
	
	unload();
}

std::vector<ResidueTorsion> Ligand::residueTorsionList()
{
	collectTorsions();
	return _rts;
}

void Ligand::grabTorsions(RTAngles &angles, rope::TorsionType type)
{
	collectTorsions();
	for (int i = 0; i < angles.size(); i++)
	{
		ResidueTorsion &rt = angles.rt(i);

		Angular f = NAN;
		for (int j = 0; j < _rts.size(); j++)
		{
			if (rt == _rts[j])
			{
				f = _rts[j].torsion().refinedAngle();
				break;
			}
		}

		angles.storage(i) = f;
	}
}
const Residue *Ligand::localResidueForResidueTorsion(const ResidueTorsion &rt)
{
	return nullptr; // ??? when we don't have a sequence then...??
}
