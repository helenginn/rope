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

#include "Model.h"
#include "Instance.h"
#include "Interface.h"
#include "AtomContent.h"
#include "BondTorsion.h"
#include "Environment.h"
#include "ModelManager.h"
#include "EntityManager.h"

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

void Instance::wipeAtoms()
{
	delete _currentAtoms;
	_currentAtoms = nullptr;
	_motherAtoms = nullptr;
}

void Instance::unload()
{
	wipeAtoms();
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

Atom *Instance::atomByIdName(const ResidueId &id, std::string name,
                             std::string chain)
{
	load();
	AtomGroup *grp = currentAtoms();
	Atom *p = grp->atomByIdName(id, name, chain);
	unload();
	
	return p;
}

Entity *Instance::entity()
{
	if (_entity != nullptr)
	{
		return _entity;
	}
	
	_entity = Environment::entityManager()->entity(_entity_id);
	return _entity;
}

void Instance::insertTransforms(AtomContent *atoms)
{
	std::map<std::string, glm::mat4x4>::iterator it;
	
	for (it = _transforms.begin(); it != _transforms.end(); it++)
	{
		std::string desc = it->first;
		glm::mat4x4 transform = it->second;
		
		Atom *a = atoms->atomByDesc(desc);
		if (a == nullptr)
		{
			std::cout << "Warning! - missing anchor definition " 
			<< desc << std::endl;
			continue;
		}
		
		a->setAbsoluteTransformation(transform);
	}
}

void Instance::extractTransformedAnchors(AtomContent *atoms)
{
	for (const Atom *anchor : atoms->transformedAnchors())
	{
		std::string desc = anchor->desc();
		glm::mat4x4 transform = anchor->transformation();
		_transforms[desc] = transform;
	}

}

const Metadata::KeyValues Instance::metadata() const
{
	Metadata::KeyValues mod = _model->metadata();
	Metadata *md = Environment::metadata();

	const Metadata::KeyValues *ptr = md->valuesForInstance(id());
	Metadata::KeyValues mol;

	if (ptr != nullptr)
	{
		mol = *ptr;
	}

	Metadata::KeyValues::const_iterator it;
	
	for (it = mol.cbegin(); it != mol.cend(); it++)
	{
		mod[it->first] = it->second;
	}
	
	return mod;
}

void Instance::updateRmsdMetadata()
{
	load();
	AtomGroup *atoms = currentAtoms();
	float val = atoms->rmsd();

	Metadata::KeyValues kv;
	kv["molecule"] = id();
	kv["rmsd"] = Value(f_to_str(val, 3));
	Environment::metadata()->addKeyValues(kv, true);
	unload();
}

void Instance::housekeeping()
{
	_model = (Environment::modelManager()->model(_model_id));
	_entity = (Environment::entityManager()->entity(_entity_id));
	
}

const Residue *Instance::localResidueForResidueTorsion(const ResidueTorsion &rt)
{
	Residue *const master = rt.residue;
	return equivalentLocal(master);
}

float Instance::valueForTorsionFromList(BondTorsion *bt,
                                        const std::vector<ResidueTorsion> &list,
                                        const std::vector<Angular> &values,
                                        std::vector<bool> &found)
{
	ResidueId target = bt->residueId();
	Residue *master = equivalentMaster(target);
	
	if (master == nullptr)
	{
		return NAN;
	}
	
	for (size_t i = 0; i < list.size(); i++)
	{
		if (list[i].entity != _entity)
		{
			continue;
		}

		Residue *residue = list[i].residue;
		
		if (residue == nullptr || residue != master)
		{
			continue;
		}

		const std::string &desc = list[i].torsion.desc();
		
		if (desc != bt->desc() && desc != bt->reverse_desc())
		{
			continue;
		}
		
		found[i] = true;
		return values[i];
	}

	return NAN;
}

void Instance::addTorsionsToGroup(MetadataGroup &group, 
                                  rope::TorsionType type)
{
	if (!isRefined())
	{
		return;
	}

	MetadataGroup::Array vals = grabTorsions(type);
	group.addMetadataArray(this, vals);
}



