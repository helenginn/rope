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

#ifndef __vagabond__Ruler__
#define __vagabond__Ruler__

#include <list>
#include "Rule.h"

#include <json/json.hpp>
using nlohmann::json;

class RulerResponder
{
public:
	virtual ~RulerResponder() {};
	virtual void objectsChanged() = 0;
};

class Ruler
{
public:
	Ruler();
	
	void setResponder(RulerResponder *responder)
	{
		_responder = responder;
	}
	
	const std::list<Rule> &rules() const
	{
		return _rules;
	}
	
	void removeLike(Rule &other);
	void addRule(Rule &rule);
	
	Rule &rule(int i)
	{
		std::list<Rule>::iterator it = _rules.begin();
		int count = 0;
		
		while (i >= 0 && count != i)
		{
			it++;
			count++;
		}
		
		return *it;
	}

	const size_t ruleCount() const
	{
		return _rules.size();
	}

	friend void to_json(json &j, const Ruler &value);
	friend void from_json(const json &j, Ruler &value);
private:
	std::list<Rule> _rules;
	RulerResponder *_responder = nullptr;
};

inline void to_json(json &j, const Ruler &value)
{
	j["rules"] = value._rules;
}

inline void from_json(const json &j, Ruler &value)
{
	try
	{
        std::list<Rule> rules = j.at("rules");
        value._rules = rules;
	}
	catch (const nlohmann::detail::out_of_range &err)
	{

	}
}

#endif
