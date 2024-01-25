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

#ifndef __vagabond__PolymerEntity__
#define __vagabond__PolymerEntity__

#include "Entity.h"
#include "TorsionData.h"
#include "PositionData.h"

class PolymerEntity : public Entity, public HasResponder<Responder<PolymerEntity> >
{
public:
	PolymerEntity();

	virtual bool hasSequence()
	{
		return true;
	}

	virtual Sequence *sequence()
	{
		return &_sequence;
	}
	
	void setSequence(Sequence *seq)
	{
		_sequence = *seq;
	}

	virtual const std::vector<Instance *> instances() const;
	
	const std::vector<Polymer *> &molecules() const
	{
		return _molecules;
	}

	const size_t moleculeCount() const
	{
		return _molecules.size();
	}

	virtual const size_t instanceCount() const
	{
		return moleculeCount();
	}

	virtual void throwOutInstance(Polymer *mol);
	virtual void appendIfMissing(Instance *mol);
	
	virtual void housekeeping();
	

	virtual Metadata *funcBetweenAtoms(const std::vector<FindAtom> &ps,
	                                   const std::string &header, 
	                                   const Calculate &calculate,
	                                   const Compare &compare);

	friend void to_json(json &j, const PolymerEntity &value);
	friend void from_json(const json &j, PolymerEntity &value);

	virtual TorsionData prepareTorsionGroup();
	virtual PositionData preparePositionGroup();
protected:

private:
	Sequence _sequence;
	std::vector<Polymer *> _molecules;

};

inline void to_json(json &j, const PolymerEntity &value)
{
	j["name"] = value._name;
	j["sequence"] = value._sequence;
	j["visuals"] = value._visPrefs;
}

inline void from_json(const json &j, PolymerEntity &value)
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
