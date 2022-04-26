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

#include "Entity.h"
#include "Model.h"

Entity::Entity()
{

}

void Entity::checkModel(Model &m)
{
	if (m.hasEntity(name()))
	{
		_models.insert(&m);

		for (Molecule &mol : m.molecules())
		{
			std::cout << m.name() << " has " << mol.chain_id() << std::endl;
			_molecules.insert(&mol);
		}
	}
}

size_t Entity::checkForUnrefinedMolecules()
{
	int count = 0;

	for (const Molecule *mol : _molecules)
	{
		bool refined = mol->isRefined();
		
		if (!refined)
		{
			count++;
		}
	}

	return count;
}

std::set<Model *> Entity::unrefinedModels()
{
	std::set<Model *> models;
	for (Molecule *mol : _molecules)
	{
		bool refined = mol->isRefined();

		if (!refined)
		{
			models.insert(mol->model());
		}
	}

	return models;
}

void Entity::refineUnrefinedModels()
{
	_refineSet = unrefinedModels();
	
	refineNextModel();
}

void Entity::refineNextModel()
{
	if (_refineSet.size() == 0)
	{
		return;
	}

	_currentModel = *_refineSet.begin();
	_refineSet.erase(_refineSet.begin());
}

void Entity::modelReady()
{

	refineNextModel();
}
