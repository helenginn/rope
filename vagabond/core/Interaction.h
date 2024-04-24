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

#ifndef __vagabond__Interaction__
#define __vagabond__Interaction__

#include <string>
#include <vector>

class Comparable;
class Model;
class Atom;

class Interaction
{
public:
	Interaction(Atom *la, Atom *ra);
	
	void setValue(const float value)
	{
		_value = value;
	}
	
	Atom *const &left() const
	{
		return _left;
	}

	Atom *const &right() const
	{
		return _right;
	}
	std::string desc() const;
	Interaction *expandedToFit(Interaction *other);
private:
	std::vector<std::string> _sides;
	float _value;
	
	Atom *_left = nullptr; /* from all model */
	Atom *_right = nullptr; /* from comparable */
};

#endif
