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

#include <set>
#include <map>
#include "Sequence.h"
#include "Substance.h"
#include "Model.h"
#include "MetadataGroup.h"
#include "PositionalGroup.h"
#include "Responder.h"
#include "VisualPreferences.h"
#include "AtomRecall.h"

#include <json/json.hpp>
using nlohmann::json;

class Molecule;

class Entity : public HasResponder<Responder<Entity>>
{
public:
	Entity();
	
	Sequence *sequence()
	{
		return &_sequence;
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

	std::set<Model *> unrefinedModels();
	size_t checkForUnrefinedMolecules();
	void throwOutMolecule(Molecule *mol);
	void throwOutModel(Model *mol);
	
	MetadataGroup makeTorsionDataGroup();
	PositionalGroup makePositionalDataGroup();
	Molecule *chooseRepresentativeMolecule();

	const size_t moleculeCount() const
	{
		return _molecules.size();
	}

	const size_t modelCount() const
	{
		return _models.size();
	}
	
	const std::vector<Molecule *> &molecules() const
	{
		return _molecules;
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
	
	VisualPreferences &visualPreferences()
	{
		return _visPrefs;
	}
	
	void setActuallyRefine(bool refine)
	{
		_actuallyRefine = refine;
	}

	friend void to_json(json &j, const Entity &value);
	friend void from_json(const json &j, Entity &value);
private:

	Sequence _sequence;
	VisualPreferences _visPrefs;
	
	Model *_currentModel = nullptr;

	void appendMolecule(Model &m);
	
	bool _actuallyRefine = true;
	std::string _name;
	
	std::set<Model *> _refineSet;
	std::vector<Model *> _models;
	std::vector<Molecule *> _molecules;
};

inline void to_json(json &j, const Entity &value)
{
	j["name"] = value._name;
	j["sequence"] = value._sequence;
	j["visuals"] = value._visPrefs;
}

inline void from_json(const json &j, Entity &value)
{
	value._name = j.at("name");
	value._sequence = j.at("sequence");

	try
	{
		value._visPrefs = j.at("visuals");
	}
	catch (...)
	{
		
	}
	
	value.clickTicker();
}

#endif
