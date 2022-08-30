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

#ifndef __vagabond__Molecule__
#define __vagabond__Molecule__

#include <vagabond/utils/glm_json.h>
using nlohmann::json;

#include <string>
#include "AtomRecall.h"
#include "HasMetadata.h"

class AtomContent;
class Entity;
class Model;
class Chain;

#include "Sequence.h"

/** \class Molecule
 * Molecule refers to a specific instance of an entity associated with a model */

class Molecule : public HasMetadata
{
public:
	Molecule(std::string model_id, std::string chain_id,
	         std::string entity_id, Sequence *derivative);
	Molecule();

	void addChain(std::string chain_id)
	{
		_chain_ids.insert(chain_id);
	}
	
	const std::set<std::string> &chain_ids() const
	{
		return _chain_ids;
	}
	
	const bool has_chain_id(std::string id) const
	{
		return _chain_ids.count(id) > 0;
	}
	
	const std::string model_chain_id() const;
	
	virtual const std::string id() const
	{
		return model_chain_id();
	}
	
	const std::string &model_id() const
	{
		return _model_id;
	}
	
	const std::string &entity_id() const
	{
		return _entity_id;
	}
	
	const bool &isRefined() const
	{
		return _refined;
	}
	
	Sequence *const sequence()
	{
		return &_sequence;
	}

	void getTorsionRefs(Chain *ch);
	void extractTorsionAngles(AtomContent *atoms);
	void extractTransformedAnchors(AtomContent *atoms);
	void insertTorsionAngles(AtomContent *atoms);

	Atom *atomByIdName(const ResidueId &id, std::string name);

	Metadata::KeyValues distanceBetweenAtoms(AtomRecall &a, AtomRecall &b,
	                                         std::string header);

	Metadata::KeyValues angleBetweenAtoms(AtomRecall &a, AtomRecall &b,
	                                      AtomRecall &c, std::string header);

	void housekeeping();
	
	void mergeWith(Molecule *b);
	
	Model *const model();
	
	void setModel(Model *model)
	{
		_model = model;
	}
	
	Entity *entity();
	
	void setEntity(Entity *entity)
	{
		_entity = entity;
	}
	
	AtomGroup *currentAtoms();
	void unload();
	
	void updateRmsdMetadata();

	float valueForTorsionFromList(BondTorsion *bt,
	                              const std::vector<ResidueTorsion> &list,
	                              const std::vector<float> &values,
	                              std::vector<bool> &found);

	virtual const Metadata::KeyValues metadata() const;

	friend void to_json(json &j, const Molecule &value);
	friend void from_json(const json &j, Molecule &value);
private:
	void harvestMutations(SequenceComparison *sc);

	std::string _model_id;
	std::string _entity_id;
	std::set<std::string> _chain_ids;

	std::map<std::string, glm::mat4x4> _transforms;
	Sequence _sequence;
	AtomGroup *_currentAtoms = nullptr;
	Model *_model = nullptr;
	Entity *_entity = nullptr;
	bool _refined = false;
};

inline void to_json(json &j, const Molecule &value)
{
	j["chain_ids"] = value._chain_ids;
	j["entity_id"] = value._entity_id;
	j["model_id"] = value._model_id;
	j["sequence"] = value._sequence;
	j["refined"] = value._refined;
	j["transforms"] = value._transforms;
}

inline void from_json(const json &j, Molecule &value)
{
	if (j.count("chain_id") > 0)
	{
        std::string chain_id = j.at("chain_id");
        value._chain_ids.insert(chain_id);
	}
	else if (j.count("chain_ids") > 0)
	{
        std::set<std::string> chain_ids = j.at("chain_ids");
		value._chain_ids = chain_ids;
	}
	value._entity_id = j.at("entity_id");
	value._model_id = j.at("model_id");
	value._sequence = j.at("sequence");
	
	try
	{
		value._refined = j.at("refined");
		value._transforms = j.at("transforms");
	}
	catch (...)
	{

	}
}

#include "Model.h"

#endif
