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

#include <json/json.hpp>
using nlohmann::json;

#include <string>
#include "HasMetadata.h"

class AtomContent;
class Entity;
class Model;
class Chain;

#include "Sequence.h"

class Molecule : public HasMetadata
{
public:
	Molecule(std::string model_id, std::string entity_id, Sequence *derivative);
	Molecule();

	void setChain(std::string chain_id)
	{
		_chain_id = chain_id;
	}
	
	const std::string &chain_id() const
	{
		return _chain_id;
	}
	
	const std::string model_chain_id() const
	{
		return _model_id + "_" + _chain_id;
	}
	
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
	void insertTorsionAngles(AtomContent *atoms);

	void housekeeping();
	
	Model *const model();
	
	void setModel(Model *model)
	{
		_model = model;
	}

	virtual const Metadata::KeyValues *metadata() const;

	friend void to_json(json &j, const Molecule &value);
	friend void from_json(const json &j, Molecule &value);
private:
	std::string _model_id;
	std::string _entity_id;
	std::string _chain_id;

	Sequence _sequence;
	Model *_model = nullptr;
	Entity *_entity = nullptr;
	bool _refined = false;
};

inline void to_json(json &j, const Molecule &value)
{
	j["chain_id"] = value._chain_id;
	j["entity_id"] = value._entity_id;
	j["model_id"] = value._model_id;
	j["sequence"] = value._sequence;
	j["refined"] = value._refined;
}

inline void from_json(const json &j, Molecule &value)
{
	value._chain_id = j.at("chain_id");
	value._entity_id = j.at("entity_id");
	value._model_id = j.at("model_id");
	value._sequence = j.at("sequence");
	
	try
	{
		value._refined = j.at("refined");
	}
	catch (...)
	{

	}
}

#include "Model.h"

#endif
