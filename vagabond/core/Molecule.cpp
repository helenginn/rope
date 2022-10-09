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

#include "Chain.h"
#include "Model.h"
#include "Value.h"
#include "Molecule.h"
#include "AtomContent.h"
#include "Environment.h"
#include "ModelManager.h"
#include "EntityManager.h"
#include "SequenceComparison.h"

Molecule::Molecule()
{

}

Molecule::Molecule(std::string model_id, std::string chain_id,
                   std::string entity_id, Sequence *derivative)
{
	_model_id = model_id;
	_entity_id = entity_id;
	_chain_ids.insert(chain_id);
	_sequence = *derivative;
	_entity = (Environment::entityManager()->entity(_entity_id));
	
	if (_entity != nullptr) // when newly made, which is when we fill this in
	{
		SequenceComparison *sc = _sequence.newComparison(_entity);
		_sequence.mapFromMaster(sc);
		harvestMutations(sc);
		delete sc;
	}

	housekeeping();
}

const std::string Molecule::model_chain_id() const
{
	std::string full = _model_id + "_";

	for (const std::string &id : _chain_ids)
	{
		full += id;
	}
	
	return full;
}

void Molecule::harvestMutations(SequenceComparison *sc)
{
	sc->calculateMutations();

	Metadata::KeyValues kv;
	kv["molecule"] = id();
	
	for (const std::string &mutation : sc->mutations())
	{
		kv["mut:" + mutation] = Value("true");
	}
	
	Environment::metadata()->addKeyValues(kv, true);
}

void Molecule::updateRmsdMetadata()
{
	AtomGroup *atoms = currentAtoms();
	float val = atoms->rmsd();

	Metadata::KeyValues kv;
	kv["molecule"] = id();
	kv["rmsd"] = Value(f_to_str(val, 3));
	Environment::metadata()->addKeyValues(kv, true);
}

void Molecule::getTorsionRefs(Chain *ch)
{
	_sequence.remapFromMaster(_entity);

	for (size_t i = 0; i < ch->bondTorsionCount(); i++)
	{
		BondTorsion *t = ch->bondTorsion(i);

		ResidueId id = t->residueId();
		TorsionRef ref = TorsionRef(t);

		Residue *local_res = _sequence.residue(id);
		if (local_res == nullptr)
		{
			continue;
		}

		local_res->addTorsionRef(ref);
		Residue *master_res = _sequence.master_residue(local_res);

		if (master_res == nullptr || master_res->nothing())
		{
			continue;
		}

		master_res->addTorsionRef(ref);
	}
}

Model *const Molecule::model()
{
	if (_model == nullptr)
	{
		_model = (Environment::modelManager()->model(_model_id));
	}
	return _model;
}

void Molecule::housekeeping()
{
	_model = (Environment::modelManager()->model(_model_id));
	_entity = (Environment::entityManager()->entity(_entity_id));
	
	_sequence.remapFromMaster(_entity);
}

void Molecule::insertTorsionAngles(AtomContent *atoms)
{
	if (!_refined)
	{
		return;
	}

	_sequence.housekeeping();

	for (const std::string &chain : _chain_ids)
	{
		Chain *ch = atoms->chain(chain);

		for (size_t i = 0; i < ch->bondTorsionCount(); i++)
		{
			BondTorsion *t = ch->bondTorsion(i);

			ResidueId id = t->residueId();
			std::string desc = t->desc();

			Residue *local = _sequence.residue(id);
			if (local == nullptr)
			{
				continue;
			}

			TorsionRef ref = local->copyTorsionRef(desc);
			if (!ref.valid())
			{
				continue;
			}

			double angle = ref.refinedAngle();
			t->setRefinedAngle(angle);
		}
	}

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

void Molecule::extractTransformedAnchors(AtomContent *atoms)
{
	for (const Atom *anchor : atoms->transformedAnchors())
	{
		std::string desc = anchor->desc();
		glm::mat4x4 transform = anchor->transformation();
		_transforms[desc] = transform;
	}

}

void Molecule::extractTorsionAngles(AtomContent *atoms, bool tmp_dest)
{
	_sequence.remapFromMaster(entity());
	for (const std::string &chain : _chain_ids)
	{
		Chain *ch = atoms->chain(chain);
		getTorsionRefs(ch);

		for (size_t i = 0; i < ch->bondTorsionCount(); i++)
		{
			BondTorsion *t = ch->bondTorsion(i);

			ResidueId id = t->residueId();
			std::string desc = t->desc();

			Residue *local = _sequence.residue(id);

			if (local == nullptr)
			{
				continue;
			}

			double angle = t->measurement(BondTorsion::SourceDerived);

			bool success = local->supplyRefinedAngle(desc, angle, tmp_dest);
			
			if (!success)
			{
				success = local->supplyRefinedAngle(t->reverse_desc(), angle, 
				                                    tmp_dest);
			}
		}
	}

	_refined = true;
}

const Metadata::KeyValues Molecule::metadata() const
{
	Metadata::KeyValues mod = _model->metadata();
	Metadata *md = Environment::metadata();

	const Metadata::KeyValues *ptr = md->valuesForMolecule(id());
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

Entity *Molecule::entity()
{
	if (_entity != nullptr)
	{
		return _entity;
	}
	
	_entity = Environment::entityManager()->entity(_entity_id);
	return _entity;
}

Atom *Molecule::atomByIdName(const ResidueId &id, std::string name)
{
	for (const std::string &chain : _chain_ids)
	{
		model();
		
		if (!_model)
		{
			return nullptr;
		}

		Chain *mine = model()->currentAtoms()->chain(chain);

		Atom *p = mine->atomByIdName(id, name);
		
		if (p != nullptr)
		{
			return p;
		}
	}
	
	return nullptr;
}

Metadata::KeyValues Molecule::distanceBetweenAtoms(AtomRecall &a, AtomRecall &b,
                                                   std::string header) 
{
	_sequence.remapFromMaster(entity());

	Metadata::KeyValues kv;
	
	Residue *local_a = _sequence.local_residue(a.master);
	Residue *local_b = _sequence.local_residue(b.master);

	if (local_a == nullptr || local_b == nullptr)
	{
		std::cout << "local_a or local_b missing" << std::endl;
		return kv;
	}
	
	Atom *p = atomByIdName(local_a->id(), a.atom_name);
	Atom *q = atomByIdName(local_b->id(), b.atom_name);

	if (!p || !q)
	{
		return kv;
	}

	std::cout << p->desc() << " vs " << q->desc() << " = ";

	glm::vec3 init_p = p->initialPosition();
	glm::vec3 init_q = q->initialPosition();
	
	double dist = glm::length(init_p - init_q);
	std::cout << dist << std::endl;
	
	kv["molecule"] = Value(id());
	kv[header] = Value(f_to_str(dist, 2));

	return kv;
}

Metadata::KeyValues Molecule::angleBetweenAtoms(AtomRecall &a, AtomRecall &b,
                                                AtomRecall &c, std::string header) 
{
	_sequence.remapFromMaster(entity());

	Metadata::KeyValues kv;
	
	Residue *local_a = _sequence.local_residue(a.master);
	Residue *local_b = _sequence.local_residue(b.master);
	Residue *local_c = _sequence.local_residue(c.master);

	if (local_a == nullptr || local_b == nullptr || local_c == nullptr)
	{
		std::cout << "local_a or local_b or local_c missing" << std::endl;
		return kv;
	}
	
	Atom *p = atomByIdName(local_a->id(), a.atom_name);
	Atom *q = atomByIdName(local_b->id(), b.atom_name);
	Atom *r = atomByIdName(local_c->id(), c.atom_name);
	
	if (!p || !q || !r)
	{
		return kv;
	}
	
	std::cout << p->desc() << " through " << q->desc() << 
	" to " << r->desc() << " = ";

	glm::vec3 init_q = q->initialPosition();
	glm::vec3 p_vec = p->initialPosition() - init_q;
	glm::vec3 r_vec = r->initialPosition() - init_q;
	p_vec = glm::normalize(p_vec);
	r_vec = glm::normalize(r_vec);
	
	double angle = rad2deg(glm::angle(p_vec, r_vec));
	std::cout << angle << std::endl;
	
	kv["molecule"] = Value(id());
	kv[header] = Value(f_to_str(angle, 2));

	return kv;
}

void Molecule::mergeWith(Molecule *b)
{
	if (_entity_id != b->_entity_id)
	{
		return;
	}

	std::cout << "Molecule " << id() << " would merge with molecule "
	<< b->id() << std::endl;
	
	int last = sequence()->lastNum();
	int gap = b->sequence()->firstNum() - last;
	
	for (size_t i = 1; i < gap; i++)
	{
		_sequence.addBufferResidue();
	}

	_sequence += b->sequence();
	_sequence.clearMaps();
	_sequence.housekeeping();
	
	for (const std::string &ch : b->chain_ids())
	{
		_chain_ids.insert(ch);
	}
	
	std::map<std::string, glm::mat4x4>::iterator it;
	
	for (it = b->_transforms.begin(); it != b->_transforms.end(); it++)
	{
		_transforms[it->first] = it->second;
	}
}

std::string get_desc(std::string name)
{
	int descptr = name.rfind(":");
	descptr++;
	if (name.size() <= descptr)
	{
		return "";
	}

	std::string desc(&name[descptr]);
	return desc;
}

Residue *Molecule::localResidueForResidueTorsion(const ResidueTorsion &rt)
{
	Residue *master = rt.residue;
	Residue *local = sequence()->local_residue(master);
	return local;
}

float Molecule::valueForTorsionFromList(BondTorsion *bt,
                                        const std::vector<ResidueTorsion> &list,
                                        const std::vector<Angular> &values,
                                        std::vector<bool> &found)
{
	ResidueId target = bt->residueId();
	Residue *local = sequence()->residueLike(target);
	Residue *master = sequence()->master_residue(local);
	
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

AtomGroup *Molecule::currentAtoms()
{
	if (_currentAtoms != nullptr)
	{
		return _currentAtoms;
	}

	if (!_model->loaded())
	{
		_model->load();
	}

	AtomContent *ac = _model->currentAtoms();
	
	AtomGroup *tmp = new AtomGroup();
	
	for (size_t i = 0; i < ac->chainCount(); i++)
	{
		Chain *candidate = ac->chain(i);
		if (has_chain_id(candidate->id()))
		{
			tmp->add(candidate);
		}
	}
	
	_currentAtoms = tmp;
	
	return tmp;
}

void Molecule::unload()
{
	delete _currentAtoms;
	_currentAtoms = nullptr;

}

MetadataGroup::Array Molecule::grabTorsions(bool tmp)
{
	sequence()->clearMaps();
	sequence()->remapFromMaster(entity());
	MetadataGroup::Array vals;

	entity()->sequence()->torsionsFromMapped(sequence(), vals, tmp);
	return vals;
}

void Molecule::addTorsionsToGroup(MetadataGroup &group)
{
	if (!isRefined())
	{
		return;
	}

	MetadataGroup::Array vals = grabTorsions();
	group.addMetadataArray(this, vals);
}
