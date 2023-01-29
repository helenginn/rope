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

class PolymerEntity : public Entity
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
	
	virtual void housekeeping();
	
	virtual Metadata *distanceBetweenAtoms(AtomRecall a, AtomRecall b);
	virtual Metadata *angleBetweenAtoms(AtomRecall a, AtomRecall b, AtomRecall c);

	friend void to_json(json &j, const PolymerEntity &value);
	friend void from_json(const json &j, PolymerEntity &value);

protected:
	virtual MetadataGroup prepareTorsionGroup();
	virtual PositionalGroup preparePositionGroup();

private:
	Sequence _sequence;

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
