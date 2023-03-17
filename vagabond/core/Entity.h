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

#include <nlohmann/json.hpp>
using nlohmann::json;

class Polymer;

class Entity : public HasResponder<Responder<Entity>>
{
public:
	Entity();
	
	virtual bool hasSequence() { return false; }

	virtual Sequence *sequence()
	{
		return nullptr;
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

	size_t unrefinedInstanceCount();
	size_t unrefinedProlineCount();
	void throwOutModel(Model *mol);

	virtual void throwOutInstance(Polymer *mol) {};
	virtual void throwOutInstance(Ligand *mol) {};
	virtual void appendIfMissing(Instance *mol) {};
	
	MetadataGroup makeTorsionDataGroup();
	PositionalGroup makePositionalDataGroup();
	Instance *chooseRepresentativeInstance();
	
	virtual const std::vector<Instance *> instances() const = 0;

	virtual const size_t instanceCount() const = 0;

	const size_t modelCount() const
	{
		return _models.size();
	}
	
	const std::vector<Model *> &models() const
	{
		return _models;
	}
	
	virtual void housekeeping();
	
	virtual Metadata *distanceBetweenAtoms(AtomRecall a, AtomRecall b)
	{
		return nullptr;
	}

	virtual Metadata *angleBetweenAtoms(AtomRecall a, AtomRecall b, AtomRecall c)
	{
		return nullptr;
	}

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

	friend void to_json(json &j, const PolymerEntity &value);
	friend void from_json(const json &j, PolymerEntity &value);
protected:
	virtual MetadataGroup prepareTorsionGroup() { return MetadataGroup(0); }
	virtual PositionalGroup preparePositionGroup() { return PositionalGroup(0); }

	VisualPreferences _visPrefs;
	
	bool _actuallyRefine = true;
	std::string _name;
	
	std::set<Model *> _refineSet;
	std::vector<Model *> _models;
};

#endif
