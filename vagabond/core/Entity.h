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

#ifndef __vagabond__Entity__
#define __vagabond__Entity__

#include <string>
#include <set>
#include <map>
#include "Sequence.h"
#include "Model.h"
#include "MetadataGroup.h"
#include "AtomRecall.h"

#include <json/json.hpp>
using nlohmann::json;

class Molecule;

class EntityResponder
{
public:
	virtual ~EntityResponder() {}
	virtual void entityDone() = 0;
	virtual void setActiveAtoms(Model *model) {}
};

class Entity : public ModelResponder
{
public:
	Entity();
	
	Sequence *sequence()
	{
		return &_sequence;
	}
	
	void setResponder(EntityResponder *responder)
	{
		_responder = responder;
	}
	
	void setSequence(Sequence *seq)
	{
		_sequence = *seq;
	}
	
	void setName(std::string name)
	{
		_name = name;
	}

	const std::string &name() const
	{
		return _name;
	}
	
	void checkModel(Model &m);
	void searchAllModels();

	size_t checkForUnrefinedMolecules();
	void refineUnrefinedModels();
	void throwOutMolecule(Molecule *mol);
	void throwOutModel(Model *mol);
	
	void refineNextModel();
	virtual void modelReady();
	
	MetadataGroup makeTorsionDataGroup();

	const size_t moleculeCount() const
	{
		return _molecules.size();
	}

	const size_t modelCount() const
	{
		return _models.size();
	}
	
	const std::vector<Model *> &models() const
	{
		return _models;
	}
	
	void housekeeping();
	
	Metadata *distanceBetweenAtoms(AtomRecall a, AtomRecall b);
	Metadata *angleBetweenAtoms(AtomRecall a, AtomRecall b, AtomRecall c);

	std::map<std::string, int> allMetadataHeaders();
	
	void clickTicker();

	friend void to_json(json &j, const Entity &value);
	friend void from_json(const json &j, Entity &value);
private:
	std::set<Model *> unrefinedModels();

	std::string _name;
	Sequence _sequence;
	
	Model *_currentModel = nullptr;
	EntityResponder *_responder = nullptr;

	void appendMolecule(Model &m);
	
	std::set<Model *> _refineSet;
	std::vector<Model *> _models;
	std::vector<Molecule *> _molecules;
};

inline void to_json(json &j, const Entity &value)
{
	j["name"] = value._name;
	j["sequence"] = value._sequence;
}

inline void from_json(const json &j, Entity &value)
{
	try
	{
		value._name = j.at("name");
	}
	catch (...)
	{
		std::cout << "Error proccessing json, probably old version" << std::endl;
	}

	try
	{
		value._sequence = j.at("sequence");
	}
	catch (...)
	{
		std::cout << "Error proccessing json, probably old version" << std::endl;
	}
	
	value.clickTicker();
}

#endif
