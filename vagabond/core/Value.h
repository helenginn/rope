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

#ifndef __vagabond__Value__
#define __vagabond__Value__

#include <iostream>

#include <json/json.hpp>
using nlohmann::json;

class Value
{
public:
	Value()
	{
	}

	Value(std::string text)
	{
		setText(text);
	}
	
	void setText(std::string text)
	{
		_text = text;
		char *end;
		_real = strtod(_text.c_str(), &end);
		
		if (end == &_text[_text.length()])
		{
			_hasNum = true;
		}
	}
	
	const bool operator==(const Value &b) const
	{
		if (_hasNum)
		{
			return fabs(_real - b._real) < 1e-6;
		}
		else
		{
			return _text == b._text;
		}
	}
	
	const bool operator<(const Value &b) const
	{
		if (_hasNum)
		{
			return _real < b._real;
		}
		else
		{
			return _text < b._text;
		}
	}
	
	const bool &hasNumber() const
	{
		return _hasNum;
	}
	
	const float &number() const
	{
		return _real;
	}

	const std::string &text() const
	{
		return _text;
	}
	
	void housekeeping()
	{
		setText(_text);
	}

	friend void to_json(json &j, const Value &value);
	friend void from_json(const json &j, Value &value);
private:
	std::string _text;
	float _real = 0.;

	bool _hasNum = false;
};

inline void to_json(json &j, const Value &value)
{
	j["text"] = value._text;
}

inline void from_json(const json &j, Value &value)
{
	value._text = j.at("text");
	
	value.housekeeping();
}

#endif
