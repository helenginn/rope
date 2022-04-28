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
#include "AtomContent.h"

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
			if (mol.entity_id() == name())
			{
				_molecules.insert(&mol);
			}
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
	std::cout << "Refine set size: " << _refineSet.size() << std::endl;
	
	refineNextModel();
}

void Entity::refineNextModel()
{
	if (_refineSet.size() == 0)
	{
		return;
	}

	_currentModel = *_refineSet.begin();
	_currentModel->setResponder(this);
	_currentModel->load();
	_refineSet.erase(_refineSet.begin());
	
	_responder->setActiveAtoms(_currentModel);
	
	_currentModel->refine();
}

void Entity::modelReady()
{
	_responder->setActiveAtoms(nullptr);
	_currentModel->setResponder(nullptr);
	refineNextModel();
}

DegreeDataGroup Entity::makeTorsionDataGroup()
{
	size_t num = _sequence.torsionCount(true);
	std::cout << "Torsion count: " << num << std::endl;
	std::vector<std::string> names;
	_sequence.addTorsionNames(names, true);

	DegreeDataGroup group(num);
	group.addUnitNames(names);
	
	for (Molecule *mol : _molecules)
	{
		Sequence *molseq = mol->sequence();
		std::cout << "Molseq = " << molseq << std::endl;
	
		molseq->clearMaps();
		molseq->remapFromMaster(this);
		std::string name = mol->model_chain_id();
		DataGroup<float>::Array vals;

		_sequence.torsionsFromMapped(molseq, vals, true);
		group.addArray(name, vals);
	}
	
	std::cout << "done" << std::endl;
	return group;
}

void Entity::throwOutModel(Model *model)
{
	size_t before = moleculeCount();
	std::set<Molecule *>::iterator it = _molecules.begin();

	while (it != _molecules.end())
	{
		Molecule *m = *it;

		if (m->model_id() == model->name())
		{
			_molecules.erase(it);
			it = _molecules.begin();
			continue;
		}
		it++;
	}

	size_t diff = before - moleculeCount();
	std::cout << "Removed " << diff << " molecules of model " << 
	model->name() << "." << std::endl;

	std::set<Model *>::iterator jt = _models.begin();
	while (jt != _models.end())
	{
		Model *m = *jt;
		if (m == model)
		{
			_models.erase(jt);
			break;
		}
		jt++;
	}

	_refineSet.clear();
	
	if (_currentModel == model)
	{
		_currentModel = nullptr;
	}
}

void Entity::throwOutMolecule(Molecule *mol)
{
	_molecules.erase(mol);

}
