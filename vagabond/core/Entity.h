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
#include "TorsionData.h"
#include "PositionData.h"
#include "Responder.h"
#include "VisualPreferences.h"

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
	
	TorsionData makeTorsionDataGroup(bool empty = false);
	PositionData makePositionalDataGroup();
	PositionData makePositionalDataGroup(std::vector<Instance *> 
	                                        &polymers);

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
	
	void setReference(Instance *const &reference)
	{
		_reference = reference;
	}
	
	virtual void housekeeping();
	
	typedef std::function<std::vector<std::pair<Atom *, Instance *>>
	(Model *model, Instance *reference)> 
	FindAtom;
	typedef std::function<float(const std::vector<Atom *> &)> Calculate;
	typedef std::function<float(const std::vector<float> &)> Compare;

	virtual Metadata *funcBetweenAtoms(const std::vector<FindAtom> &ps,
	                                   const std::string &header, 
	                                   const Calculate &calculate,
	                                   const Compare &compare)
	{
		return nullptr;
	}
	
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

	virtual TorsionData prepareTorsionGroup() { return TorsionData(0); }
	virtual PositionData preparePositionGroup() { return PositionData(0); }
protected:
	Instance *_reference = nullptr;

	VisualPreferences _visPrefs;
	
	bool _actuallyRefine = true;
	std::string _name;
	
	std::set<Model *> _refineSet;
	std::vector<Model *> _models;
};

#endif
