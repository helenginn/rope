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
#include "Chain.h"
#include "Model.h"
#include "Value.h"
#include "Polymer.h"
#include "RTAngles.h"
#include "Superpose.h"
#include "HyperValue.h"
#include "AtomContent.h"
#include "Environment.h"
#include "ModelManager.h"
#include "EntityManager.h"
#include "PolymerEntity.h"
#include "SequenceComparison.h"

Polymer::Polymer()
{

}

Polymer::Polymer(std::string model_id, std::string chain_id,
                   std::string entity_id, Sequence *derivative)
{
	_model_id = model_id;
	_entity_id = entity_id;
	_chain_ids.insert(chain_id);
	_sequence = *derivative;
	_entity = (Environment::entityManager()->entity(_entity_id));
	
	if (_entity != nullptr) // when newly made, which is when we fill this in
	{
		SequenceComparison *sc = _sequence.newComparison();
		_sequence.mapFromMaster(sc);
		harvestMutations(sc);
		delete sc;
	}

	housekeeping();
}

const std::string Polymer::chain_id_str() const
{
	std::string full;
	for (const std::string &id : _chain_ids)
	{
		full += id;
	}

	return full;
}

const std::string Polymer::model_chain_id() const
{
	std::string full = _model_id + "_" + chain_id_str();
	return full;
}

void Polymer::harvestMutations(SequenceComparison *sc)
{
	sc->calculateMutations();

	Metadata::KeyValues kv;
	kv["molecule"] = id();
	kv["chain"] = chain_id_str();
	
	for (const std::string &mutation : sc->mutations())
	{
		kv["mut:" + mutation] = Value("true");
	}
	
	Environment::metadata()->addKeyValues(kv, true);
}

void Polymer::putTorsionRefsInSequence(Chain *ch)
{
	for (size_t i = 0; i < ch->parameterCount(); i++)
	{
		Parameter *p = ch->parameter(i);

		ResidueId id = p->residueId();
		TorsionRef ref = TorsionRef(p);

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

void Polymer::housekeeping()
{
	Instance::housekeeping();
}

void Polymer::insertTorsionAngles(AtomContent *atoms)
{
	if (!isRefined())
	{
		return;
	}

	_sequence.housekeeping();

	for (const std::string &chain : _chain_ids)
	{
		Chain *ch = atoms->chain(chain);

		for (size_t i = 0; i < ch->parameterCount(); i++)
		{
			Parameter *p = ch->parameter(i);

			ResidueId id = p->residueId();
			std::string desc = p->desc();

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

			p->setValue(angle);
		}
	}
	
	insertTransforms(atoms);
}

void Polymer::extractTorsionAngles(AtomContent *atoms, bool tmp_dest)
{
	for (const std::string &chain : _chain_ids)
	{
		Chain *ch = atoms->chain(chain);
		putTorsionRefsInSequence(ch);

		for (size_t i = 0; i < ch->parameterCount(); i++)
		{
			Parameter *p = ch->parameter(i);

			ResidueId id = p->residueId();
			std::string desc = p->desc();

			Residue *local = _sequence.residue(id);

			if (local == nullptr)
			{
				continue;
			}

			double angle = p->empiricalMeasurement();

			bool success = local->supplyRefinedAngle(desc, angle, tmp_dest);
		}
	}

	setRefined(true);
	setProlined(true);
}

void Polymer::mergeWith(Polymer *b)
{
	if (_entity_id != b->_entity_id)
	{
		return;
	}

	std::cout << "Polymer " << id() << " would merge with molecule "
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

Residue *const Polymer::equivalentMaster(const ResidueId &target)
{
	Sequence *seq = sequence();
	Residue *local = seq->residueLike(target);
	Residue *const master = seq->master_residue(local);

	return master;
}

Residue *const Polymer::equivalentLocal(const ResidueId &m_id)
{
	Residue *master = polymerEntity()->sequence()->residueLike(m_id);
	return equivalentLocal(master);
}

Residue *Polymer::localForLocalId(const ResidueId &l_id)
{
	Residue *local = sequence()->residueLike(l_id);
	return local;
}

Residue *const Polymer::equivalentLocal(Residue *const master) 
{
	Sequence *seq = sequence();
	Residue *const local = seq->local_residue(master);
	return local;
}

bool Polymer::atomBelongsToInstance(Atom *a)
{
	return has_chain_id(a->chain()) && localForLocalId(a->residueId());
}

void Polymer::grabTorsions(RTAngles &angles, rope::TorsionType type)
{
	sequence()->torsionsFromMapped(angles, type);
}

std::vector<ResidueTorsion> Polymer::residueTorsionList()
{
	std::vector<ResidueTorsion> rts;
	polymerEntity()->sequence()->addResidueTorsions(rts);
	
	for (ResidueTorsion &rt : rts)
	{
		rt.attachToInstance(this);
	}
	return rts;
}

std::map<Atom *, Atom *> Polymer::mapAtoms(Polymer *other)
{
	std::map<Atom *, Atom *> map;
	std::map<ResidueId, ResidueId> resMap;

	for (Residue &r : sequence()->residues())
	{
		Residue *master = sequence()->master_residue(&r);
		if (!master) continue;

		Residue *local = other->sequence()->local_residue(master);
		if (!local) continue;

		resMap[r.id()] = local->id();
	}
	
	AtomGroup *myAtoms = currentAtoms();
	AtomGroup *otherAtoms = other->currentAtoms();
	
	for (Atom *a : myAtoms->atomVector())
	{
		ResidueId target = a->residueId();
		ResidueId local = resMap[target];
		
		Atom *pair = otherAtoms->atomByIdName(local, a->atomName());
		if (pair)
		{
			map[a] = pair;
		}
	}
	
	return map;
}

std::vector<Posular> Polymer::atomPositionList(Instance *reference,
                           const std::vector<Atom3DPosition> &headers,
                           std::map<ResidueId, int> &resIdxs)
{
	if (hasAtomPositionList(reference))
	{
		return _inst2Pos[reference];
	}

	model()->load(Model::NoGeometry);

	std::map<Atom *, Atom *> atomMap = reference->mapAtoms(this);

	Superpose sp;
	for (auto it = atomMap.begin(); it != atomMap.end(); it++)
	{
		glm::vec3 fixed = it->first->initialPosition();
		glm::vec3 moving = it->second->initialPosition();

		sp.addPositionPair(fixed, moving);
	}

	sp.superpose();
	glm::mat4 tr = sp.transformation();

	std::vector<Posular> vex(headers.size(), glm::vec3(NAN));
	for (auto it = atomMap.begin(); it != atomMap.end(); it++)
	{
		Atom *moving = it->second;
		glm::vec3 one = moving->initialPosition();
		one = glm::vec3(tr * glm::vec4(one, 1.f));

		ResidueId local_id = moving->residueId();
		Residue *res = sequence()->residueLike(local_id);
		if (!res) continue;
		Residue *master = sequence()->master_residue(res);
		if (!master) continue;
		ResidueId id = master->id();

		if (resIdxs.count(id) == 0) { continue; }

		int start = resIdxs[id];
		while (start < headers.size() && 
		       headers[start].master()->id() == id)
		{
			if (moving->atomName() == headers[start].atomName())
			{
				vex[start] = one;
			}
			start++;
		}
	}

	_inst2Pos[reference] = vex;

	model()->unload();

	return vex;
}

Atom *Polymer::equivalentForAtom(Polymer *other, Atom *atom)
{
	AtomGroup *otherAtoms = other->currentAtoms();
	AtomGroup *myAtoms = currentAtoms();
	
	// get local residue out of the sequence of that molecule
	Residue *local = other->sequence()->residueLike(atom->residueId());

	// convert into the master residue for the entity
	Residue *master = other->sequence()->master_residue(local);

	// convert master residue into local entity for this molecule
	Residue *myLocal = sequence()->local_residue(master);

	if (myLocal == nullptr) return nullptr;
	
	// grab same atom name out of this molecule's atomgroup
	Atom *chosen = myAtoms->atomByIdName(myLocal->id(), atom->atomName());
	return chosen;
}

PolymerEntity *const Polymer::polymerEntity() const
{
	return static_cast<PolymerEntity *>(_entity);
}

const size_t Polymer::completenessScore() const
{
	return sequence()->modelledResidueCount();
}

Atom *Polymer::atomForIdentifier(const Atom3DPosition &pos)
{
	Residue *myLocal = sequence()->local_residue(pos.master());
	if (!myLocal)
	{
		return nullptr;
	}
	Atom *chosen = currentAtoms()->atomByIdName(myLocal->id(), pos.atomName());
	return chosen;
}
