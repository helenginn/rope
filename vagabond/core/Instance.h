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

#ifndef __vagabond__Instance__
#define __vagabond__Instance__

/** \class Instance group of atoms corresponding to a concrete/observed
 *  molecule from a structure */

#include <vagabond/utils/glm_json.h>
#include <vagabond/c4x/Angular.h>
#include <vagabond/c4x/Posular.h>

#include "ResidueTorsion.h"
#include "HasMetadata.h"
#include "ResidueId.h"
#include "RopeTypes.h"

using nlohmann::json;

class Atom3DPosition;

class ResidueTorsion;
class AtomContent;
class BFactorData;
class TorsionData;
class Parameter;
class Interface;
class AtomGroup;
class RAFloats;
class RTAngles;
class Polymer;
class Residue;
class Entity;
class Ligand;
class Model;
class Atom;

class Instance : public HasMetadata 
{
public:
	Instance();
	virtual ~Instance() {};
	
	Model *const model();

	void setModel(Model *model)
	{
		_model = model;
	}
	
	virtual bool hasSequence() const = 0;
	
	const std::string &model_id() const
	{
		return _model_id;
	}

	virtual bool displayable() const
	{
		return true;
	}
	
	void housekeeping();
	
	virtual const std::string id() const { return ""; }
	
	virtual std::string desc() const { return ""; }
	
	/** generate a simple interface between this comparable and its model
	 *  with a tight cutoff distance */
	Interface *interfaceWithOther(Instance *other, float threshold = 4);

	void addToInterface(Instance *other, Interface *face, 
	                    double max, bool derived);

	/* gets atoms from MY atoms corresponding to other's atom */
	Atom *equivalentForAtom(Instance *other, Atom *atom);
	virtual Atom *equivalentForAtom(Polymer *other, Atom *atom)
	{
		return nullptr;
	}

	virtual Atom *atomForIdentifier(const Atom3DPosition &pos);

	virtual Atom *equivalentForAtom(Ligand *other, Atom *atom)
	{
		return nullptr;
	}
	
	void superposeOn(Instance *other);
	
	virtual const Residue *
	localResidueForResidueTorsion(const ResidueTorsion &rt);

	virtual Residue *const equivalentMaster(const ResidueId &local)
	{ return nullptr; }
	virtual Residue *const equivalentLocal(Residue *const master)
	{ return nullptr; }
	virtual Residue *const equivalentLocal(const ResidueId &m_id)
	{ return nullptr; }
	virtual Residue *localForLocalId(const ResidueId &l_id)
	{ return nullptr; }

	int indexForParameterFromList(Parameter *param, const RTAngles &list);
	float valueForTorsionFromList(Parameter *bt, const RTAngles &list);

	virtual const Metadata::KeyValues metadata(Metadata *from = nullptr) const;
	void updateRmsdMetadata();
	
	virtual std::map<Atom *, Atom *> mapAtoms(Polymer *other)
	{
		return std::map<Atom *, Atom *>();
	}

	Atom *atomByIdName(const ResidueId &id, std::string name, 
	                   std::string chain = "");
	
	void load();
	int loadCount();
	void setAtomGroupSubset();
	virtual AtomGroup *currentAtoms();
	void wipeAtoms();
	bool unload();
	void reload();

	Entity *entity();
	
	void setEntity(Entity *entity)
	{
		_entity = entity;
	}
	
	const std::string &entity_id() const
	{
		return _entity_id;
	}

	virtual bool isProlined() const
	{
		return true;
	}
	
	const bool &isRefined() const
	{
		return _refined;
	}

	void insertTransforms(AtomContent *atoms);
	void extractTransformedAnchors(AtomContent *atoms);
	
	virtual const size_t completenessScore() const
	{
		return 0;
	}

	virtual void insertTorsionAngles(AtomContent *atoms) {};
	virtual void extractTorsionAngles(AtomContent *atoms, 
	                                  bool tmp_dest = false) {}
	
	friend void to_json(json &j, const Polymer &value);
	friend void from_json(const json &j, Polymer &value);

	virtual std::vector<ResidueTorsion> residueTorsionList(bool add_hydrogens 
	                                                       = false)
	{
		return std::vector<ResidueTorsion>();
	}
	virtual bool atomBelongsToInstance(Atom *a) = 0;
	virtual void grabTorsions(RTAngles &angles,
	                          rope::TorsionType type = rope::RefinedTorsions) {}

	bool hasAtomPositionList(Instance *reference)
	{
		return (_inst2Pos.count(reference) > 0);
	}

	virtual std::vector<Posular> atomPositionList(Instance *reference,
	                                              const std::vector<Atom3DPosition>
	                                              &headers,
	                                              std::map<ResidueId, int> 
	                                              &resIdxs)
	{
		return std::vector<Posular>();
	};

	void addTorsionsToGroup(TorsionData &group, rope::TorsionType type);
	void addBFactorsToGroup(BFactorData &group);
protected:
	void grabBFactors(RAFloats &bVals);

	std::string _model_id;
	std::string _entity_id;

	Model *_model = nullptr;
	Entity *_entity = nullptr;

	AtomGroup *_currentAtoms = nullptr;
	AtomGroup *_motherAtoms = nullptr;
	
	void setRefined(bool r)
	{
		_refined = r;
	}

	std::map<Instance *, std::vector<Posular> > _inst2Pos;
private:

	bool _refined = false;
	std::map<std::string, glm::mat4x4> _transforms;
};

#endif
