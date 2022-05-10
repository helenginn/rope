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

#include "Ruler.h"

Ruler::Ruler()
{

}

void Ruler::removeLike(Rule &other)
{
	Rule copy = other;
	std::list<Rule>::iterator it = _rules.begin();

	for (Rule &r : _rules)
	{
		if (copy == r)
		{
			_rules.erase(it);
			break;
		}
		
		it++;
	}
	
	if (_responder)
	{
		_responder->objectsChanged();
	}
}

void Ruler::addRule(Rule &rule)
{
	_rules.push_back(rule);
	
	if (_responder)
	{
		_responder->objectsChanged();
	}
}

