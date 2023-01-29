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
#include "Model.h"

Ligand::Ligand(std::string model_id, AtomGroup *grp)
{
	_model_id = model_id;
	_currentAtoms = nullptr;
	_anchorDesc = grp->chosenAnchor()->desc();
	_nickname = grp->chosenAnchor()->code();
	_chain = grp->chosenAnchor()->chain();
	
	for (const Atom *a : grp->atomVector())
	{
		const ResidueId &id = a->residueId();
		_resids.insert(id);
	}
}

Ligand::Ligand()
{

}

std::string Ligand::desc() const
{
	std::string d = _model_id + ":" + _chain + "/" + _nickname;
	
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
