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

#include "Instance.h"
#include "Interface.h"
#include "AtomGroup.h"
#include "Model.h"
#include "Environment.h"
#include "ModelManager.h"

Instance::Instance()
{

}

void Instance::addToInterface(Instance *other, Interface *face, 
                              double max, bool derived)
{
	AtomGroup *left = other->currentAtoms();
	AtomGroup *right = currentAtoms();

	for (Atom *l : left->atomVector())
	{
		if (right->hasAtom(l))
		{
			continue;
		}

		glm::vec3 lv = derived ? l->derivedPosition() : l->initialPosition();

		for (Atom *r : right->atomVector())
		{
			glm::vec3 rv = derived ? r->derivedPosition() : r->initialPosition();
			glm::vec3 diff = rv - lv;

			float length = glm::length(diff);
			
			if (length > max)
			{
				continue;
			}
			
			Interaction ia(l, r);
			ia.setValue(length);
			face->addInteraction(ia);
		}
	}
	
	unload();
}

void Instance::load()
{
	model()->load();

	if (_motherAtoms == _model->currentAtoms() && _currentAtoms != nullptr)
	{
		// return pre-calculated subset if model has not been reloaded
		return;
	}

	setAtomGroupSubset();
}

AtomGroup *Instance::currentAtoms()
{
	if (model()->currentAtoms() && _currentAtoms == nullptr)
	{
		setAtomGroupSubset();
	}

	if (_motherAtoms != model()->currentAtoms() && _currentAtoms != nullptr)
	{
		// recalculate subset as model atoms have changed
		setAtomGroupSubset();
	}

	return _currentAtoms;
}

Model *const Instance::model()
{
	if (_model == nullptr)
	{
		_model = (Environment::modelManager()->model(_model_id));
	}
	return _model;
}

void Instance::unload()
{
	delete _currentAtoms;
	_currentAtoms = nullptr;

	_model->unload();
}


Interface *Instance::interfaceWithOther(Instance *other)
{
	Interface *face = new Interface(other, this);
	addToInterface(other, face, 4, false);
	std::cout << face->desc() << std::endl;

	return face;
}

void Instance::setAtomGroupSubset()
{
	AtomContent *ac = _model->currentAtoms();
	_motherAtoms = ac;
	
	AtomGroup *tmp = new AtomGroup();
	
	for (size_t i = 0; i < ac->size(); i++)
	{
		Atom *candidate = (*ac)[i];
		
		if (atomBelongsToInstance(candidate))
		{
			*tmp += candidate;
		}

	}
	
	_currentAtoms = tmp;
}
