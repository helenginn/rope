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
#include "files/PdbFile.h"
#include "Superpose.h"
#include "AtomContent.h"
#include "Parameter.h"
#include "Environment.h"
#include "ModelManager.h"
#include "EntityManager.h"
#include "Atom3DPosition.h"

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
		if (right->hasAtom(l) || l->elementSymbol() == "H")
		{
			continue;
		}

		glm::vec3 lv = derived ? l->derivedPosition() : l->initialPosition();

		for (Atom *r : right->atomVector())
		{
			if (r->elementSymbol() == "H")
			{
				continue;
			}

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
}

void Instance::load()
{
	model()->load();

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
	if (_currentAtoms)
	{
		delete _currentAtoms;
		_currentAtoms = nullptr;
		_motherAtoms = nullptr;
	}
}

bool Instance::unload()
{
	bool happened = _model->unload();
	
	if (happened)
	{
		wipeAtoms();
	}
	
	return happened;
}

void Instance::reload()
{
	_model->reload();
	wipeAtoms();
	setAtomGroupSubset();
}


Interface *Instance::interfaceWithOther(Instance *other, float threshold)
{
	Interface *face = new Interface(other, this);
	addToInterface(other, face, threshold, false);

	return face;
}

void Instance::setAtomGroupSubset()
{
	if (_motherAtoms == model()->currentAtoms() && _currentAtoms != nullptr)
	{
		// return pre-calculated subset if model has not been reloaded
		return;
	}

	AtomContent *ac = model()->currentAtoms();
	_motherAtoms = ac;
	
	AtomGroup *tmp = new AtomGroup();
	tmp->setGrabsBondstraints(true);
	
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

const Metadata::KeyValues Instance::metadata(Metadata *from) const
{
	Metadata *md = from ? from : Environment::metadata();
	Metadata::KeyValues mod = _model->metadata(md);

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
	Residue *const master = rt.master();
	return equivalentLocal(master);
}

int Instance::indexForParameterFromList(Parameter *param, const RTAngles &list)
{
	ResidueId target = param->residueId();
	Residue *master = equivalentMaster(target);

	if (master == nullptr)
	{
		return -1;
	}
	
	for (size_t i = 0; i < list.size(); i++)
	{
		const ResidueTorsion &rt = list.rt(i);
		if (!rt.fitsParameter(param, this))
		{
			continue;
		}
		
		return i;
	}

	return -1;
}

float Instance::valueForTorsionFromList(Parameter *param, const RTAngles &list)
{
	int idx = indexForParameterFromList(param, list);
	
	if (idx < 0)
	{
		return NAN;
	}

	return list.storage(idx);
}

void Instance::addTorsionsToGroup(TorsionData &group, rope::TorsionType type)
{
	if (!isRefined())
	{
		return;
	}

	RTAngles angles = group.emptyAngles(true);
	grabTorsions(angles, type);
	group.addMetadataArray(this, angles);
}

void Instance::superposeOn(Instance *other)
{
	AtomGroup *otherAtoms = other->currentAtoms();
	AtomGroup *myAtoms = currentAtoms();
	
	Superpose sp;
	for (Atom *a : myAtoms->atomVector())
	{
		a->setOtherPosition("original", glm::vec3(NAN));
	}

	for (Atom *theirs : otherAtoms->atomVector())
	{
		Atom *mine = equivalentForAtom(other, theirs);
		
		if (mine)
		{
			glm::vec3 t = theirs->derivedPosition();
			glm::vec3 d = mine->derivedPosition();
			mine->setOtherPosition("original", t);
			sp.addPositionPair(t, d);
		}
	}

	sp.superpose();
	glm::mat4 tr = sp.transformation();

	for (Atom *mine : myAtoms->atomVector())
	{
		glm::vec3 d = mine->derivedPosition();
		glm::vec3 update = glm::vec3(tr * glm::vec4(d, 1.f));
		mine->setDerivedPosition(update);
	}
}

Atom *Instance::equivalentForAtom(Instance *other, Atom *atom)
{
	if (other->hasSequence())
	{
		Polymer *p = static_cast<Polymer *>(other);	
		return equivalentForAtom(p, atom);
	}
	else
	{
		Ligand *l = static_cast<Ligand *>(other);	
		return equivalentForAtom(l, atom);
	}
}

int Instance::loadCount()
{
	return model()->loadCount();
}

Atom *Instance::atomForIdentifier(const Atom3DPosition &pos)
{
	return nullptr;
}

void Instance::grabBFactors(RAFloats &bVals)
{
	AtomGroup *grp = currentAtoms();

	RAFloats localValues;
	float total_b = 0;
	float count = 0;

	for (Atom *atom : grp->atomVector())
	{
		Residue *master = equivalentMaster(atom->residueId());
		
		if (!master)
		{
			continue;
		}
		
		Atom3DPosition a3p(master, atom->atomName());
		float b = atom->initialBFactor();
		total_b += b;
		count++;
		
		localValues.addAtom3DPosition(a3p, b);
	}
	
	total_b /= count;
	
	std::vector<DataFloat> tmp = localValues.storage_according_to(bVals);
	
	for (DataFloat &f : tmp)
	{
		f /= total_b;
	}

	std::vector<Atom3DPosition> headers = bVals.headers_only();
	bVals.vector_from(headers, tmp);
}

void Instance::addBFactorsToGroup(BFactorData &group)
{
	model()->load(Model::NoGeometry);
	AtomGroup *grp = currentAtoms();
	
	RAFloats bVals = group.emptyBFactors(true);
	grabBFactors(bVals);
	group.addMetadataArray(this, bVals.storage_only());

	model()->unload();
}
