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
#include "Metadata.h"
#include "AtomContent.h"

Entity::Entity()
{

}

std::set<std::string> Entity::allMetadataHeaders()
{
	std::set<std::string> headers;
	
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
			headers.insert(it->first);
		}
	}

	return headers;
}

Metadata *Entity::distanceBetweenAtoms(Residue *master_id_a, std::string a_name,
                                       Residue *master_id_b, std::string b_name)
{
	Metadata *md = new Metadata();
	
	std::string header;
	header += master_id_a->id().as_string() + a_name;
	header += " to ";
	header += master_id_b->id().as_string() + b_name;

	for (Molecule *molecule : _molecules)
	{
		Metadata::KeyValues kv;
		molecule->sequence()->remapFromMaster(this);
		kv = molecule->distanceBetweenAtoms(master_id_a, a_name,
		                                    master_id_b, b_name, header);

		if (kv.size() == 0)
		{
			continue;
		}

		md->addKeyValues(kv, true);
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
	
	_responder->setActiveAtoms(_currentModel);
	
	_currentModel->refine();
}

void Entity::modelReady()
{
	_responder->setActiveAtoms(nullptr);
	_currentModel->setResponder(nullptr);
	refineNextModel();
}

MetadataGroup Entity::makeTorsionDataGroup()
{
	size_t num = _sequence.torsionCount(true);
	std::cout << "Torsion count: " << num << std::endl;
	std::vector<std::string> names;
	_sequence.addTorsionNames(names, true);

	MetadataGroup group(num);
	group.addUnitNames(names);
	
	for (Molecule *mol : _molecules)
	{
		if (!mol->isRefined())
		{
			continue;
		}

		std::cout << "order " << mol->id() << std::endl;
		Sequence *molseq = mol->sequence();
		molseq->clearMaps();
		molseq->remapFromMaster(this);
		MetadataGroup::Array vals;

		_sequence.torsionsFromMapped(molseq, vals, true);
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
