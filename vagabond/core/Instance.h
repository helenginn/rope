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
#include "HasMetadata.h"
#include <vagabond/utils/glm_json.h>
using nlohmann::json;

class AtomContent;
class Interface;
class AtomGroup;
class Molecule;
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
	
	friend void to_json(json &j, const Molecule &value);
	friend void from_json(const json &j, Molecule &value);
protected:
	virtual bool atomBelongsToInstance(Atom *a) = 0;

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
private:
	void setAtomGroupSubset();

	bool _refined = false;
	std::map<std::string, glm::mat4x4> _transforms;

};

#endif
