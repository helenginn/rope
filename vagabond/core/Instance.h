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

#include "ResidueId.h"
#include "RopeTypes.h"
#include "HasMetadata.h"
#include "MetadataGroup.h"
#include <vagabond/utils/glm_json.h>
#include <vagabond/c4x/Angular.h>
#include <vagabond/c4x/Posular.h>
using nlohmann::json;

struct ResidueTorsion;
class AtomContent;
class BondTorsion;
class Interface;
class AtomGroup;
class Molecule;
class Residue;
class Entity;
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
	Interface *interfaceWithOther(Instance *other);

	void addToInterface(Instance *other, Interface *face, 
	                    double max, bool derived);

	virtual Atom *equivalentForAtom(Model *other, std::string desc)
	{
		return nullptr;
	}
	
	const Residue *localResidueForResidueTorsion(const ResidueTorsion &rt);
	virtual Residue *const equivalentMaster(const ResidueId &local)
	{ return nullptr; }
	virtual Residue *const equivalentLocal(Residue *const master) const
	{ return nullptr; }
	virtual Residue *const equivalentLocal(const ResidueId &m_id) const
	{ return nullptr; }

	float valueForTorsionFromList(BondTorsion *bt,
	                              const std::vector<ResidueTorsion> &list,
	                              const std::vector<Angular> &values,
	                              std::vector<bool> &found);

	virtual const Metadata::KeyValues metadata() const;
	void updateRmsdMetadata();
	
	virtual std::map<Atom *, Atom *> mapAtoms(Molecule *other)
	{
		return std::map<Atom *, Atom *>();
	}

	Atom *atomByIdName(const ResidueId &id, std::string name, 
	                   std::string chain = "");
	
	void load();
	virtual AtomGroup *currentAtoms();
	void unload();

	Entity *entity();
	
	void setEntity(Entity *entity)
	{
		_entity = entity;
	}
	
	const std::string &entity_id() const
	{
		return _entity_id;
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

	virtual void extractTorsionAngles(AtomContent *atoms, bool tmp_dest) {}
	
	friend void to_json(json &j, const Molecule &value);
	friend void from_json(const json &j, Molecule &value);

	virtual bool atomBelongsToInstance(Atom *a) = 0;
	virtual MetadataGroup::Array grabTorsions(rope::TorsionType type)
	{
		return MetadataGroup::Array();
	}

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

	void addTorsionsToGroup(MetadataGroup &group, rope::TorsionType type);
protected:

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
	void setAtomGroupSubset();

	bool _refined = false;
	std::map<std::string, glm::mat4x4> _transforms;
};

#endif
