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

#ifndef __vagabond__Rule__
#define __vagabond__Rule__

#include <string>

#include <json/json.hpp>
using nlohmann::json;

enum Scheme
{
	BlueOrange,
	pHIndicator,
	Nothing = 0,
};

// map TaskState values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM( Scheme,
                             {
	                            {Nothing, nullptr},
	                            {BlueOrange, "blueorange"},
	                            {pHIndicator, "phindicator"},
                             })

class Rule
{
public:
	enum Type
	{
		LineSeries,
		ChangeIcon,
		VaryColour,
	};
	
	Rule(Type type = LineSeries);
	
	const bool operator==(const Rule &other) const
	{
		return (_type == other._type && _header == other._header);
	}
	
	void setType(const Type type)
	{
		_type = type;
	}
	
	const Type &type() const
	{
		return _type;
	}
	
	void setScheme(Scheme scheme)
	{
		_scheme = scheme;
	}
	
	const Scheme &scheme() const
	{
		return _scheme;
	}

	void setHeader(std::string header)
	{
		_header = header;
	}
	
	const std::string &header() const
	{
		return _header;
	}
	
	const std::string desc() const;
	
	const float &max() const
	{
		return _maxVal;
	}
	
	void setMax(float max)
	{
		_maxVal = max;
	}
	
	void setMin(float min)
	{
		_minVal = min;
	}
	
	const float &min() const
	{
		return _minVal;
	}
	
	void setVals(std::vector<float> &vals);
	
	void convert_value(float &val) const
	{
		float v = val;
		val = (v - _minVal) / (_maxVal - _minVal);
	}
	
	enum Label
	{
		None = 0,
		Start = 1 << 0,
		End = 1 << 1,
	};
	
	const Label &label() const
	{
		return _label;
	}
	
	void setLabel(Label l)
	{
		_label = l;
	}
private:
	Type _type = LineSeries;
	Scheme _scheme = BlueOrange;
	Label _label = None;
	
	float _minVal = 4;
	float _maxVal = 6.8;

	std::vector<float> _vals;

	std::string _header;

};

#endif
