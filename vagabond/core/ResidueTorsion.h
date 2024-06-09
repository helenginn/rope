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

#ifndef __vagabond__ResidueTorsion__
#define __vagabond__ResidueTorsion__

#include "BitIdentifier.h"
#include "TorsionRef.h"

/** \class ResidueTorsion
 *  holds the identity of a Residue/Torsion pair + entity and can interconvert
 *  between master and local residue when supplied with an instance */

class ResidueTorsion : public BitIdentifier
{
public:
	ResidueTorsion() = default;
	ResidueTorsion(Parameter *const &param);
	virtual ~ResidueTorsion() {};
	virtual void housekeeping();
	
	const TorsionRef &torsion() const
	{
		return _torsion;
	}
	
	void setTorsion(const TorsionRef &torsion)
	{
		_torsion = torsion;
	}
	
	const bool operator==(const ResidueTorsion &other) const
	{
		bool first = (_master == other._master && _torsion == other._torsion
		              && _entityName == other._entityName);
		
		bool second = (!_instance || !other._instance 
		               || _instance == other._instance);
		
		return first && second;
	}
	
	Parameter *parameter();
	
	bool fitsParameter(Parameter *other, Instance *from) const;

	std::string desc() const;
	std::string status() const;

	friend void to_json(json &j, const ResidueTorsion &value);
	friend void from_json(const json &j, ResidueTorsion &value);
private:
	TorsionRef _torsion{};
};

inline void to_json(json &j, const ResidueTorsion &id)
{
	j["entity"] = id._entityName;
	j["torsion"] = id._torsion;
	
	if (id._masterSet)
	{
		j["master"] = id._masterId;
	}
	
	if (id._localSet)
	{
		j["local"] = id._localId;
	}
	
}

inline void from_json(const json &j, ResidueTorsion &id)
{
	j.at("entity").get_to(id._entityName);
	j.at("torsion").get_to(id._torsion);
	
	if (j.count("master"))
	{
		j.at("master").get_to(id._masterId);
		id._masterSet = true;
	}
	
	if (j.count("local"))
	{
		j.at("local").get_to(id._localId);
		id._localSet = true;
	}
}

#endif
