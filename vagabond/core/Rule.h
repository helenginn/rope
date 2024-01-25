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
#include <iostream>
#include <vagabond/utils/glm_import.h>

#include <nlohmann/json.hpp>
using nlohmann::json;

enum Scheme
{
	BlueOrange,
	OrangeWhitePurple,
	Heat,
	Cluster4x,
	Nothing = 0,
};

// map TaskState values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM( Scheme,
                             {
	                            {Nothing, nullptr},
	                            {BlueOrange, "blueorange"},
	                            {OrangeWhitePurple, "orangewhitepurple"},
	                            {Cluster4x, "cluster4x"},
	                            {Heat, "heat"},
                             })

class HasMetadata;
class ObjectData;
class Metadata;

class Rule
{
public:
	enum Type
	{
		LineSeries,
		ChangeIcon,
		VaryColour,
	};

	// map TaskState values to JSON as strings
	NLOHMANN_JSON_SERIALIZE_ENUM( Type,
	                             {
		                            {LineSeries, "lineseries"},
		                            {ChangeIcon, "changeicon"},
		                            {VaryColour, "varycolour"},
	                             })

	enum Label
	{
		None = 0,
		Start = 1 << 0,
		End = 1 << 1,
	};

	// map TaskState values to JSON as strings
	NLOHMANN_JSON_SERIALIZE_ENUM( Label,
	                             {
		                            {None, "none"},
		                            {Start, "start"},
		                            {End, "end"},
		                            {Start | End, "both"},
	                             })
	

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

	void setMetadata(Metadata *md)
	{
		_md = md;
	}

	void setHeader(std::string header)
	{
		_header = header;
		_headerValue = "";
	}
	
	const std::string &header() const
	{
		return _header;
	}
	
	const std::string &headerValue() const
	{
		return _headerValue;
	}

	void setHeaderValue(std::string value)
	{
		_headerValue = value;
	}
	
	const std::string desc() const;
	
	const std::string exactDesc() const;
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
	
	std::vector<float> valuesForObjects(ObjectData *group) const;
	
	void convert_value(float &val) const
	{
		float v = val;
		val = (v - _minVal) / (_maxVal - _minVal);
	}
	
	const bool &ifAssigned() const
	{
		return _assigned;
	}
	
	void setAssigned(bool assign)
	{
		_assigned = assign;
		if (assign)
		{
			_headerValue = "";
		}
	}
	
	void setPointType(int idx)
	{
		_pointType = idx;
	}
	
	const int &pointType() const
	{
		return _pointType;
	}
	
	const Label &label() const
	{
		return _label;
	}
	
	void setLabel(Label l)
	{
		_label = l;
	}
	
	bool appliesToObject(HasMetadata *hm) const;

	friend void to_json(json &j, const Rule &value);
	friend void from_json(const json &j, Rule &value);
private:
	Type _type = LineSeries;
	Scheme _scheme = BlueOrange;
	Label _label = None;
	
	float _minVal = 4;
	float _maxVal = 6.8;
	bool _assigned = true;

	std::vector<float> _vals;

	std::string _header;
	std::string _headerValue;
	
	Metadata *_md = nullptr;

	int _pointType = 1;
};

inline void to_json(json &j, const Rule &value)
{
	j["type"] = value._type;
	j["scheme"] = value._scheme;
	j["label"] = value._label;
	j["min"] = value._minVal;
	j["max"] = value._maxVal;
	j["assigned"] = value._assigned;
	j["header"] = value._header;
	j["headervalue"] = value._headerValue;
	j["point_type"] = value._pointType;
}

inline void from_json(const json &j, Rule &value)
{
	try
	{
		value._type = j.at("type");
		value._scheme = j.at("scheme");
		value._label = j.at("label");
		value._minVal = j.at("min");
		value._maxVal = j.at("max");
		value._assigned = j.at("assigned");
		value._header = j.at("header");
		value._headerValue = j.at("headervalue");
		value._pointType = j.at("point_type");
	}
	catch (const nlohmann::detail::out_of_range &err)
	{

	}
}


#endif
