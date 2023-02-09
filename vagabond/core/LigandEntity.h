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

#ifndef __vagabond__LigandEntity__
#define __vagabond__LigandEntity__

#include "Entity.h"
#include "Responder.h"

class Ligand;
class Instance;

class LigandEntity : public Entity, public HasResponder<Responder<LigandEntity> >
{
public:
	LigandEntity();
	
	virtual const std::vector<Instance *> instances() const;

	const std::vector<Ligand *> &molecules() const
	{
		return _ligands;
	}

	const size_t ligandCount() const
	{
		return _ligands.size();
	}

	virtual const size_t instanceCount() const
	{
		return ligandCount();
	}

	virtual void throwOutInstance(Ligand *mol);
	virtual void appendIfMissing(Instance *mol);
	
	virtual void housekeeping();

	friend void to_json(json &j, const LigandEntity &value);
	friend void from_json(const json &j, LigandEntity &value);
private:
	
	std::vector<Ligand *> _ligands;
};

inline void to_json(json &j, const LigandEntity &value)
{
	j["name"] = value._name;
	j["visuals"] = value._visPrefs;
};

inline void from_json(const json &j, LigandEntity &value)
{
	value._name = j.at("name");
	value._visPrefs = j.at("visuals");
	
	value.clickTicker();
}

#endif
