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

#ifndef __vagabond__TorsionRef__
#define __vagabond__TorsionRef__

#include "BondTorsion.h"
#include "ResidueId.h"

#include <iostream>
#include <nlohmann/json.hpp>
using nlohmann::json;

class TorsionRef
{
public:
	TorsionRef();
	TorsionRef(std::string desc)
	{
		_desc = desc;
		organiseDescriptions();
	}

	float tmpAngle()
	{
		return _tmpAngle;
	}
	
	void setTmpAngle(float angle)
	{
		_tmpAngle = angle;
	}

	const float &refinedAngle() const
	{
		return _refinedAngle;
	}
	
	void setRefinedAngle(float angle)
	{
		_refinedAngle = angle;
	}

	TorsionRef(Parameter *tmp);
	
	const std::string &desc() const
	{
		return _desc;
	}
	
	bool isHyperParameter() const
	{
		return (atomCount() == 1);
	}
	
	bool coversMainChain() const;
	
	const bool valid() const
	{
		return (_desc.length() > 0);
	}
	
	void organiseDescriptions();
	void housekeeping();

	friend void to_json(json &j, const TorsionRef &value);
	friend void from_json(const json &j, TorsionRef &value);
	
	const bool operator==(const std::string &desc) const
	{
		return _desc == desc || _reverse_desc == desc;
	}
	
	const bool operator==(const TorsionRef &other) const
	{
		return _desc == other._desc || _reverse_desc == other._desc;
	}
	
	const bool operator<(const TorsionRef &other) const
	{
		return _desc < other._desc;
	}

	std::string atomName(int i) const;
	
	size_t atomCount() const;
private:
	std::string _desc;
	std::string _reverse_desc;
	float _refinedAngle = 0;
	float _tmpAngle = 0;
	Parameter *_parameter = nullptr;
};

inline void to_json(json &j, const TorsionRef &value)
{
	j["desc"] = value._desc;
	j["angle"] = value._refinedAngle;
}

inline void from_json(const json &j, TorsionRef &value)
{
	try
	{
		value._desc = j.at("desc");
		value._refinedAngle = j.at("angle");
	}
	catch (...)
	{

	}
	
	value.housekeeping();
}

#endif

