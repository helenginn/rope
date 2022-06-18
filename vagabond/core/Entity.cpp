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
#include "Environment.h"
#include "EntityManager.h"
#include "ModelManager.h"
#include "Model.h"
#include "Metadata.h"
#include "AtomContent.h"

Entity::Entity()
{

}

std::map<std::string, int> Entity::allMetadataHeaders()
{
	std::map<std::string, int> headers;
	
	for (const Molecule *molecule : _molecules)
	{
		const Metadata::KeyValues kv = molecule->metadata();
		
		if (kv.size() == 0)
		{
			continue;
		}

		Metadata::KeyValues::const_iterator it;
		
		for (it = kv.cbegin(); it != kv.cend(); it++)
		{
			headers[it->first]++;
		}
	}

	return headers;
}

Metadata *Entity::angleBetweenAtoms(AtomRecall a, AtomRecall b, AtomRecall c)
{
	Metadata *md = new Metadata();
	
	std::string header;
	header += a.master->id().as_string() + a.atom_name;
	header += " through ";
	header += b.master->id().as_string() + b.atom_name;
	header += " to ";
	header += c.master->id().as_string() + c.atom_name;
	
	AtomRecall new_a = AtomRecall(_sequence.residueLike(a.master->id()),
	                              a.atom_name);
	AtomRecall new_b = AtomRecall(_sequence.residueLike(b.master->id()),
	                              b.atom_name);
	AtomRecall new_c = AtomRecall(_sequence.residueLike(c.master->id()),
	                              c.atom_name);

	for (Model *model : _models)
	{
		model->angleBetweenAtoms(this, new_a, new_b, new_c, header, md);
	}

	return md;
}

Metadata *Entity::distanceBetweenAtoms(AtomRecall a, AtomRecall b)
{
	Metadata *md = new Metadata();
	
	std::string header;
	header += a.master->id().as_string() + a.atom_name;
	header += " to ";
	header += b.master->id().as_string() + b.atom_name;
	
	AtomRecall new_a = AtomRecall(_sequence.residueLike(a.master->id()),
	                              a.atom_name);
	AtomRecall new_b = AtomRecall(_sequence.residueLike(b.master->id()),
	                              b.atom_name);

	for (Model *model : _models)
	{
		model->distanceBetweenAtoms(this, new_a, new_b, header, md);
	}

	return md;
}


void Entity::checkModel(Model &m)
{
	if (m.hasEntity(name()))
	{
		m.appendIfMissing(_models);

		for (Molecule &mol : m.molecules())
		{
			if (mol.entity_id() == name())
			{
				mol.appendIfMissing(_molecules);
			}
		}
	}
	
	triggerResponse();
}

const bool compare_id(const Molecule *a, const Molecule *b)
{
	return a->id() > b->id();
}

void Entity::housekeeping()
{
	std::sort(_molecules.begin(), _molecules.end(), compare_id);
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
	
	sendResponse("model", _currentModel);
	
	_currentModel->refine();
}

void Entity::respond()
{
	sendResponse("model_done", nullptr);
	_currentModel->removeResponder(this);
	_currentModel->unload();
	refineNextModel();
}

MetadataGroup Entity::makeTorsionDataGroup()
{
	const bool only_main = true;
	size_t num = _sequence.torsionCount(only_main);
	std::cout << "Torsion count: " << num << std::endl;
	std::vector<std::string> names;
	_sequence.addTorsionNames(names, only_main);

	MetadataGroup group(num);
	group.addUnitNames(names);
	
	for (Molecule *mol : _molecules)
	{
		if (!mol->isRefined())
		{
			continue;
		}

		Sequence *molseq = mol->sequence();
		molseq->clearMaps();
		molseq->remapFromMaster(this);
		MetadataGroup::Array vals;

		_sequence.torsionsFromMapped(molseq, vals, only_main);
		group.addMetadataArray(mol, vals);
	}
	
	std::cout << "done" << std::endl;
	return group;
}

void Entity::throwOutModel(Model *model)
{
	size_t before = moleculeCount();
	std::vector<Molecule *>::iterator it = _molecules.begin();

	while (it != _molecules.end())
	{
		Molecule *m = *it;

		if (m->model_id() == model->name())
		{
			m->eraseIfPresent(_molecules);
			it = _molecules.begin();
			continue;
		}

		it++;
	}

	size_t diff = before - moleculeCount();
	std::cout << "Removed " << diff << " molecules of model " << 
	model->name() << "." << std::endl;

	std::vector<Model *>::iterator jt = _models.begin();
	while (jt != _models.end())
	{
		Model *m = *jt;
		if (m == model)
		{
			m->eraseIfPresent(_models);
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
	mol->eraseIfPresent(_molecules);
}

void Entity::clickTicker()
{
	Environment::entityManager()->clickTicker();
}
