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

#include "Comparable.h"
#include "Interface.h"
#include "AtomGroup.h"
#include "Model.h"
#include "Environment.h"
#include "ModelManager.h"

Comparable::Comparable()
{

}

void Comparable::addToInterface(Interface *face, Comparable *other,
                                double max, bool derived)
{
	model()->load();
	AtomGroup *left = model()->currentAtoms();
	AtomGroup *right = other->currentAtoms();

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
			
			Interaction ia(this, other, l, r);
			ia.setValue(length);
			face->addInteraction(ia);
		}
	}
}

Model *const Comparable::model()
{
	if (_model == nullptr)
	{
		_model = (Environment::modelManager()->model(_model_id));
	}
	return _model;
}


Interface *Comparable::interfaceWithComparable(Comparable *other)
{
	Interface *face = new Interface(this, other);
	addToInterface(face, other, 4, false);
	std::cout << face->desc() << std::endl;

	return face;
}
