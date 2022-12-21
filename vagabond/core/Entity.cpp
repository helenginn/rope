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
#include "PathManager.h"
#include "Model.h"
#include "Metadata.h"
#include "AtomContent.h"
#include "PositionalGroup.h"

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
		md->clickTicker();
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
		md->clickTicker();
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
	_sequence.setEntity(this);
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

void Entity::refineAllModels()
{
	_refineSet.clear();
	
	for (Model *m : _models)
	{
		_refineSet.insert(m);
	}

	std::cout << "Refine set size: " << _refineSet.size() << std::endl;
	
	refineNextModel();
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
		triggerResponse();
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

	triggerResponse();
}

void Entity::throwOutMolecule(Molecule *mol)
{
	mol->eraseIfPresent(_molecules);
}

void Entity::clickTicker()
{
	Environment::entityManager()->clickTicker();
}

Molecule *Entity::chooseRepresentativeMolecule()
{
	Molecule *best = nullptr;
	size_t best_count = 0;

	for (Molecule *m : _molecules)
	{
		size_t count = m->sequence()->modelledResidueCount();
		
		if (count > best_count)
		{
			best = m;
			best_count = count;
		}
	}
	
	return best;
}

MetadataGroup Entity::makeTorsionDataGroup()
{
	size_t num = _sequence.torsionCount();
	std::vector<ResidueTorsion> headers;
	_sequence.addResidueTorsions(headers);

	MetadataGroup group(num);
	group.addHeaders(headers);
	
	for (Molecule *mol : _molecules)
	{
		mol->addTorsionsToGroup(group);
	}
		
	PathManager *pm = Environment::env().pathManager();

	for (Molecule *mol : _molecules)
	{
		std::vector<Path *> paths = pm->pathsForMolecule(mol);

		for (Path *path : paths)
		{
			if (path->contributesToSVD())
			{
				path->addTorsionsToGroup(group);
			}
		}
	}
	
	return group;
}

PositionalGroup Entity::makePositionalDataGroup()
{
	std::vector<Atom3DPosition> headers;
	_sequence.addAtomPositionHeaders(headers);
	
	PositionalGroup group(headers.size());
	group.addHeaders(headers);

	/* make a quick lookup table for the first residue in each */
	std::map<ResidueId, int> resIdxs;
	
	Residue *last = nullptr;
	for (size_t i = 0; i < headers.size(); i++)
	{
		if (headers[i].residue != last)
		{
			resIdxs[headers[i].residue->id()] = i;
			last = headers[i].residue;
		}
	}
	
	Molecule *reference = chooseRepresentativeMolecule();
	reference->currentAtoms()->recalculate();
	
	for (Model *m : _models)
	{
		for (Molecule &mm : m->molecules())
		{
			std::vector<Posular> vex = mm.atomPositionList(reference,
			                                               headers, resIdxs);
			group.addMetadataArray(&mm, vex);
		}
		
		m->unload();
	}

	return group;
}
