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
}

Ligand::Ligand()
{

}

std::string Ligand::desc() const
{
	return _model_id + "(" + _chain + "/" + _nickname + ")";
}

void Ligand::housekeeping()
{
	if (_currentAtoms != nullptr)
	{
		return;
	}
	
	ModelManager *mm = Environment::modelManager();
	Model *m = mm->model(_model_id);
	
	if (!m)
	{
		return;
	}
	
	bool isLoaded = m->loaded();

	if (!isLoaded)
	{
		m->load();
	}
	
	AtomGroup *contents = m->currentAtoms();
	contents->connectedGroups();
	Atom *anchor = contents->atomByDesc(_anchorDesc);
	
	AtomGroup *mine = contents->connectedGroupToAnchor(anchor);
	_currentAtoms = mine;
}
