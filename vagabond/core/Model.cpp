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

#include <vagabond/utils/version.h>
#include "Model.h"
#include "ModelManager.h"
#include "Polymer.h"
#include "Chain.h"
#include "files/File.h"
#include "AtomContent.h"
#include "Environment.h"
#include "grids/Diffraction.h"
#include "EntityManager.h"
#include "SequenceComparison.h"
#include "../utils/FileReader.h"

Model::Model()
{
	_loadMutex = new std::mutex();
}

void Model::setFilename(std::string file)
{
	_filename = file;
	
	if (_name.length() == 0)
	{
		_name = getBaseFilename(_filename);
	}

}

const std::string Model::entityForChain(std::string id) const
{
	if (_chain2Entity.count(id))
	{
		return _chain2Entity.at(id);
	}

	return "";
}

void Model::swapChainToEntity(std::string id, std::string entity)
{

	std::cout << "Switching entity for polymer." << std::endl;
	if (_chain2Polymer.count(id))
	{
		Polymer *pol = _chain2Polymer[id];
		if (pol != nullptr)
		{
			std::cout << "Found old polymer of entity " << 
			pol->entity_id() << std::endl;
			std::cout << "Purging old polymer from environment" << std::endl;
			_chain2Entity.erase(id);
		}
	}

	std::cout << "Assigning new entity, " << entity << std::endl;
	_chain2Entity[id] = entity;
}

void Model::setEntityForChain(std::string id, std::string entity)
{
	if (_chain2Entity.count(id) && _chain2Entity[id] != entity)
	{
		swapChainToEntity(id, entity);
	}
	else
	{
		_chain2Entity[id] = entity;
	}
}

bool Model::hasEntity(std::string entity) const
{
	std::map<std::string, std::string>::const_iterator it;
	
	for (it = _chain2Entity.cbegin(); it != _chain2Entity.cend(); it++)
	{
		if (it->second == entity)
		{
			return true;
		}
	}
	
	return false;
}

Polymer *Model::polymerFromChain(Chain *ch)
{
	std::string entity = entityForChain(ch->id());
	if (entity == "")
	{
		return nullptr;
	}

	Polymer mc(name(), ch->id(), entity, ch->fullSequence());
	_polymers.push_back(mc);

	Polymer &ref = _polymers.back();

	ref.addChain(ch->id());
	ref.putTorsionRefsInSequence(ch);
	
	return &_polymers.back();
}

void Model::write(std::string filename)
{
	if (_currentFile)
	{
		_currentFile->write(filename);
	}
}

void Model::reload()
{
	if (loaded())
	{
		int last = _loadCounter;
		_loadCounter = 1;
		unload();
		load();
		_loadCounter = last;
	}
	else
	{
		load();
	}

}

bool Model::unload()
{
	std::unique_lock<std::mutex> lock(*_loadMutex);

	if (_loadCounter == 0) { return false; } // don't unload twice!
	_loadCounter--;
//	std::cout << "Model " << name() << " load counter: " 
//	<< _loadCounter << std::endl;
	if (_loadCounter > 0) { return false; }
	
	if (_currentFile)
	{
		delete _currentFile;
		_currentFile = nullptr;
	}
	
	if (_currentAtoms)
	{
		delete _currentAtoms;
		_currentAtoms = nullptr;
	}
	
	std::vector<Instance *> insts = instances();
	for (Instance *inst : insts)
	{
		inst->wipeAtoms();
	}

	return true;
}

Instance *Model::instanceWithId(std::string id)
{
	for (Instance *inst : instances())
	{
		if (inst->id() == id)
		{
			return inst;
		}
	}

	return nullptr;
}

void Model::createPolymers()
{
	load();

	std::map<std::string, std::string>::iterator it;
	int extra = 0;
	
	for (it = _chain2Entity.begin(); it != _chain2Entity.end(); it++)
	{
		std::string id = it->first;
		if (_chain2Polymer.count(id) > 0)
		{
			continue;
		}
		
		load();

		Chain *ch = _currentAtoms->chain(id);
		
		if (ch)
		{
			polymerFromChain(ch);
			extra++;
		}
	}
	
	mergeAppropriatePolymers();
	
	unload();
}

std::set<Entity *> Model::entities()
{
	std::set<Entity *> entities;

	std::vector<Instance *> insts = instances();
	for (Instance *inst : insts)
	{
		if (inst->entity() != nullptr)
		{
			entities.insert(inst->entity());
		}
	}
	
	return entities;
}

std::set<Polymer *> Model::polymersForEntity(Entity *ent)
{
	std::set<Polymer *> ps;
	for (Polymer &p : _polymers)
	{
		if (p.entity_id() == ent->name())
		{
			ps.insert(&p);
		}
	}

	return ps;
}

bool Model::mergePolymersInSet(std::set<Polymer *> polymers)
{
	float best_distance = FLT_MAX;
	Polymer *best_a = nullptr;
	Polymer *best_b = nullptr;

	for (Polymer *a : polymers)
	{
		for (Polymer *b : polymers)
		{
			if (a == b)
			{
				continue;
			}

			Sequence *seq_a = a->sequence();
			Sequence *seq_b = b->sequence();
			
			if (!seq_a || !seq_b) { continue; }
			
			if (!(seq_b->firstNum() > seq_a->lastNum() &&
			      seq_b->lastNum() > seq_a->lastNum()) ||
			     (seq_a->firstNum() > seq_b->lastNum() &&
			      seq_a->lastNum() > seq_b->lastNum()))
			{
				// overlapping sequences
				continue;
			}
			
			Sequence *first = (seq_b->firstNum() < seq_a->lastNum() ?
			                   seq_b : seq_a);
			Sequence *last = (seq_b->firstNum() < seq_a->lastNum() ?
			                  seq_a : seq_b);
			
			ResidueId id0 = first->residue(first->size() - 1)->id();
			ResidueId id1 = last->residue(0)->id();

			// now we need to decide which pair to merge; this will
			// be the pair where the N- and C-termini come closest!
			
			load(NoGeometry); // if not previously done
			
			// we need to find an atom in these residue ranges
			
			Atom *aAtom = a->atomByIdName(first == seq_a ? id0 : id1, "");
			Atom *bAtom = b->atomByIdName(first == seq_a ? id1 : id0, "");
			
			if (aAtom == nullptr|| bAtom == nullptr)
			{
				std::cout << "atoms not found" << std::endl;
				continue;
			}
			
			glm::vec3 diff = aAtom->initialPosition() - bAtom->initialPosition();
			float d = glm::length(diff);
			
			if (d < best_distance)
			{
				best_distance = d;
				if (first == seq_a)
				{
					best_a = a;
					best_b = b;
				}
				else
				{
					best_b = a;
					best_a = b;
				}
			}
		}
	}

	if (best_a != nullptr && best_b != nullptr)
	{
		best_a->mergeWith(best_b);
		throwOutInstance(best_b);
		return true;
	}

	return false;
}

void Model::mergeAppropriatePolymers()
{
	std::set<Entity *> ents = entities();
	bool restart = true;
	
	while (restart)
	{
		restart = false;
		for (Entity *entity : ents)
		{
			std::set<Polymer *> pols = polymersForEntity(entity);

			if (pols.size() <= 1)
			{
				continue;
			}

			if (mergePolymersInSet(pols))
			{
				restart = true;
				break;
			}
		}
	}
}

void update_score_if_better(Sequence *compare, Entity &ent, 
                            float &best_match, Entity **best_entity)
{
	Sequence *master = ent.sequence();

	SequenceComparison *sc = new SequenceComparison(master, compare);
	float match = sc->match();

	if (match > best_match)
	{
		best_match = match;
		*best_entity = &ent;
	}

	delete sc;
}

void Model::assignClutter()
{
	for (AtomGroup *grp : _currentAtoms->connectedGroups())
	{
		if (!grp->chosenAnchor()->hetatm())
		{
			continue;
		}
		
		bool found = false;
		for (Ligand &l : _ligands)
		{
			if (l.atomBelongsToInstance(grp->chosenAnchor()))
			{
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			Ligand lig(_name, grp);
			_ligands.push_back(lig);
		}
	}

	std::cout << "Total ligands: " << _ligands.size() << std::endl;
}

void Model::assignSequencedPolymers(Entity *chosen)
{
	EntityManager *eManager = Environment::entityManager();
	for (size_t i = 0; i < _currentAtoms->chainCount(); i++)
	{
		float best_match = 0;
		Entity *best_entity = nullptr;
		Chain *ch = _currentAtoms->chain(i);
		if (_chain2Entity.count(_currentAtoms->chain(i)->id()))
		{
			continue;
		}
		
		if (ch->sequence()->size() <= 1)
		{
			continue;
		}

		Sequence *compare = ch->fullSequence();
		
		for (size_t i = 0; i < eManager->objectCount() && chosen == nullptr; i++)
		{
			Entity &ent = eManager->object(i);

			if (compare->size() <= 10 && ent.sequence()->size() > 20)
			{
				continue;
			}

			update_score_if_better(compare, ent, best_match, &best_entity);
		}
		
		if (chosen != nullptr)
		{
			update_score_if_better(compare, *chosen, best_match, &best_entity);
		}

		if (best_match > 0.8)
		{
			std::cout << "Chain " << ch->id() << " in model " << name() <<
			 " becomes " << best_entity->name() << std::endl;
			setEntityForChain(ch->id(), best_entity->name());
		}
		else
		{
			std::cout << "Chain " << ch->id() << " of " << ch->sequence()->size()
			<< " remains unassigned" << std::endl;
		}
	}
}

void Model::autoAssignEntities(Entity *chosen)
{
	load(NoAngles);

	assignSequencedPolymers(chosen);
	
#ifdef VERSION_LIGANDS
	assignClutter();
#endif
	
	unload();
	housekeeping();
}

const Metadata::KeyValues Model::metadata() const
{
	Metadata *md = Environment::metadata();
	const Metadata::KeyValues *ptr = md->values(_name, _filename);
	
	if (ptr != nullptr)
	{
		return *ptr;
	}

	return Metadata::KeyValues();
}

void Model::housekeeping()
{
	for (Polymer &pol : _polymers)
	{
		for (const std::string &ch : pol.chain_ids())
		{
			_chain2Polymer[ch] = &pol;
		}

		pol.setModel(this);
		
	}
	
	if (_chain2Polymer.size() >= _chain2Entity.size())
	{
		return;
	}

	createPolymers();
	
	std::set<Entity *> ents = entities();
	for (Entity *ent : ents)
	{
		ent->checkModel(*this);
	}
}

void Model::findInteractions()
{
	if (_ligands.size() > 0 && _polymers.size() > 0)
	{
		_ligands.front().interfaceWithOther(&_polymers.front());
	}
}

void Model::insertTorsions()
{
	std::vector<Instance *> insts = instances();
	for (Instance *inst : insts)
	{
		inst->insertTorsionAngles(_currentAtoms);
	}
}

void Model::extractTorsions()
{
	std::cout << "Extracting torsions from model " << name() << std::endl;
	load();
	std::vector<Instance *> insts = instances();
	for (Instance *inst : insts)
	{
		inst->extractTorsionAngles(_currentAtoms);
		inst->extractTransformedAnchors(_currentAtoms);
		inst->updateRmsdMetadata();
	}
	unload();
}

void Model::load(LoadOptions opts)
{
	std::unique_lock<std::mutex> lock(*_loadMutex);

	_loadCounter++;

	if (_loadCounter > 1)
	{
		return;
	}

	_currentFile = File::openUnknown(_filename);
	
	if (opts == NoGeometry)
	{
		_currentFile->setAutomaticKnot(File::KnotNone);
	}
	else if (opts == NoAngles)
	{
		_currentFile->setAutomaticKnot(File::KnotLengths);
	}

	_currentFile->parse();
	_currentAtoms = _currentFile->atoms();
	_currentAtoms->setResponder(this);
	
	if (opts == Everything)
	{
		insertTorsions();
	}
	
	std::vector<Instance *> insts = instances();
	for (Instance *inst : insts)
	{
		inst->setAtomGroupSubset();
	}

	lock.unlock();
}

void Model::refine(bool sameThread, bool thorough)
{
	if (_currentAtoms == nullptr)
	{
		load();
	}
	
	std::vector<Instance *> insts = instances();
	for (Instance *inst : insts)
	{
		AtomGroup *grp = inst->currentAtoms();
		grp->recalculate();
		grp->refinePositions(sameThread, thorough);
	}

	extractExisting();
}

void Model::extractExisting()
{
	insertTorsions();
	extractTorsions();
	HasResponder<Responder<Model>>::triggerResponse();
}

size_t Model::instanceCountForEntity(std::string entity_id) const
{
	size_t count = 0;

	for (const Polymer &pol : _polymers)
	{
		if (pol.entity_id() == entity_id)
		{
			count++;
		}
	}

	for (const Ligand &lig : _ligands)
	{
		if (lig.entity_id() == entity_id)
		{
			count++;
		}
	}

	return count;
}

void Model::finishedRefinement()
{
	extractTorsions();
	HasResponder<Responder<Model>>::triggerResponse();
}

Model Model::autoModel(std::string filename)
{
	Model m;
	m.setFilename(filename);

	return m;
}

void Model::throwOutInstance(Instance *inst)
{
	{
		std::list<Polymer>::iterator it = _polymers.begin();

		for (Polymer &pol : _polymers)
		{
			if (inst->id() == pol.id())
			{
				_polymers.erase(it);
				return;
			}

			it++;
		}
	}

	{
		for (Ligand &l : _ligands)
		{
			std::list<Ligand>::iterator it = _ligands.begin();
			if (inst->id() == l.id())
			{
				_ligands.erase(it);
				return;
			}

			it++;
		}
	}
}

void Model::throwOutEntity(Entity *ent)
{
	std::map<std::string, std::string> newMap;

	for (auto jt = _chain2Entity.begin(); jt != _chain2Entity.end(); jt++)
	{
		if (jt->second == ent->name())
		{
			std::cout << "Removing " << jt->first << " as instance of " << 
			"entity " << ent->name() << " from " << name() << std::endl;
		}
		else
		{
			newMap[jt->first] = jt->second;
		}
	}
	
	_chain2Entity = newMap;

//	std::list<Polymer>::iterator it = _polymers.begin();

	for (auto it = _polymers.begin(); it != _polymers.end(); )
	{
		Polymer &p = *it;
		std::cout << "Checking " << p.entity_id() << " name " << 
		p.id() << std::endl;
		if (p.entity_id() == ent->name())
		{
			for (const std::string &chain : p.chain_ids())
			{
				_chain2Polymer.erase(chain);
				std::cout << "Found polymer " << chain << " to remove "
				"from " << name() << std::endl;
			}

			_polymers.erase(it);
			it = _polymers.begin();
			continue;
		}
		it++;
	}
}

void Model::clickTicker()
{
	Environment::modelManager()->clickTicker();
}

void Model::respond()
{
	extractTorsions();
}

void Model::export_refined(std::string prefix, std::string suffix)
{
    load();
    _currentAtoms->recalculate();

    std::string model_path = "";
    if (!prefix.empty())
    {
        model_path += prefix + "_";
    }
    model_path += id();
    if (!suffix.empty())
    {
        model_path += "_" + suffix;
    }
    model_path += ".pdb";
    write(model_path);

    unload();
}

std::vector<Instance *> Model::instances()
{
	std::vector<Instance *> instances;
	instances.reserve(_polymers.size() + _ligands.size());

	for (Polymer &p : _polymers)
	{
		instances.push_back(&p);
	}

	for (Ligand &l : _ligands)
	{
		instances.push_back(&l);
	}
	
	return instances;
}

Diffraction *Model::getDiffraction(std::string filename)
{
	if (filename == "")
	{
		filename = _dataFile;
	}

	File *file = File::loadUnknown(filename);

	if (!file)
	{
		return nullptr;
	}

	File::Type type = file->cursoryLook();

	if (type & File::Reflections)
	{
		Diffraction *diff = file->diffractionData();
		delete file;
		return diff;
	}

	delete file;
	return nullptr;
}


