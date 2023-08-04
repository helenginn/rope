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
#include "PositionalGroup.h"

Entity::Entity()
{

}

std::map<std::string, int> Entity::allMetadataHeaders()
{
	std::map<std::string, int> headers;
	
	for (const Instance *instance : instances())
	{
		const Metadata::KeyValues kv = instance->metadata();
		
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

void Entity::checkModel(Model &m)
{
	if (m.hasEntity(name()))
	{
		m.appendIfMissing(_models);

		for (Instance *inst : m.instances())
		{
			if (inst->entity_id() == name())
			{
				appendIfMissing(inst);
			}
		}
	}
	
	triggerResponse();
}

void Entity::housekeeping()
{

}

size_t Entity::unrefinedProlineCount()
{
	int count = 0;

	for (const Instance *inst : instances())
	{
		if (!inst->isRefined())
		{
			continue;
		}

		bool refined = inst->isProlined();
		
		if (!refined)
		{
			count++;
		}
	}

	return count;
	
}

size_t Entity::unrefinedInstanceCount()
{
	int count = 0;

	for (const Instance *inst : instances())
	{
		bool refined = inst->isRefined() && inst->isProlined();
		
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
	for (Instance *inst : instances())
	{
		bool refined = inst->isRefined() && inst->isProlined();

		if (!refined)
		{
			models.insert(inst->model());
		}
	}

	return models;
}

void Entity::throwOutModel(Model *model)
{
	size_t before = instanceCount();
	std::vector<Instance *> insts = instances();
	std::vector<Instance *>::iterator it = insts.begin();

	while (it != insts.end())
	{
		Instance *i = *it;

		if (i->model_id() == model->name())
		{
			Polymer *p = static_cast<Polymer *>(i);
			insts.erase(it);
			throwOutInstance(p);
			it = insts.begin();
			continue;
		}

		it++;
	}

	size_t diff = before - instanceCount();
	std::cout << "Removed " << diff << " polymers of model " << 
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

	triggerResponse();
}

void Entity::clickTicker()
{
	Environment::entityManager()->forPolymers()->clickTicker();
}

Instance *Entity::chooseRepresentativeInstance()
{
	if (_reference)
	{
		return _reference;
	}
	Instance *best = nullptr;
	Instance *highest = nullptr;
	size_t best_count = 0;
	float highest_res = FLT_MAX;

	for (Instance *i : instances())
	{
		size_t count = i->completenessScore();
		
		if (count > best_count)
		{
			best = i;
			best_count = count;
		}
	}
	
	if (highest != nullptr)
	{
		best = highest;
	}
	
	std::cout << "Choosing reference: " << best->id() << std::endl;
	_reference = best;
	
	return best;
}

MetadataGroup Entity::makeTorsionDataGroup(bool empty)
{
	MetadataGroup group = prepareTorsionGroup();
	
	if (empty)
	{
		return group;
	}
	
	if (!Environment::modelManager()->tryLock())
	{
		throw std::runtime_error("Busy modifying models, please wait");
	}

	for (Instance *inst : instances())
	{
		inst->addTorsionsToGroup(group, rope::RefinedTorsions);
	}
		
	Environment::modelManager()->unlock();
	return group;
}

PositionalGroup Entity::makePositionalDataGroup()
{
	std::vector<Instance *> polymers;
	for (Model *m : _models)
	{
		for (Instance &mm : m->polymers())
		{
			polymers.push_back(&mm);
		}
	}

	return makePositionalDataGroup(polymers);
}

PositionalGroup Entity::makePositionalDataGroup(std::vector<Instance *> 
                                                &polymers)
{
	PositionalGroup group = preparePositionGroup();
	const std::vector<Atom3DPosition> &headers = group.headers();

	/* make a quick lookup table for the first residue in each */
	std::map<ResidueId, int> resIdxs;
	
	Residue *last = nullptr;
	for (size_t i = 0; i < headers.size(); i++)
	{
		if (headers[i].master() != last)
		{
			resIdxs[headers[i].master()->id()] = i;
			last = headers[i].master();
		}
	}
	
	Instance *reference = chooseRepresentativeInstance();
	reference->load();
	reference->currentAtoms()->recalculate();
	
	for (Instance *mm : polymers)
	{
		std::vector<Posular> vex = mm->atomPositionList(reference,
		                                               headers, resIdxs);
		group.addMetadataArray(mm, vex);
	}

	reference->unload();

	return group;
}
