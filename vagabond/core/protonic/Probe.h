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

#ifndef __vagabond__Probe__
#define __vagabond__Probe__

#define Z_DEF (-50)
#include <vagabond/core/Atom.h>

class Probe
{
public:
	virtual const glm::vec3 &position() const
	{
		return _pos;
	}
	
	virtual ~Probe()
	{

	}
	
	void setPosition(const glm::vec2 &position)
	{
		_pos = glm::vec3(position, Z_DEF);
	}

	glm::vec3 _pos = {};
	glm::vec3 _init = {};
};

class AtomProbe : public Probe
{
public:
	AtomProbe(hnet::AtomConnector &obj, Atom *inherit = nullptr) : _obj(obj)
	{
		if (inherit)
		{
			_atom = inherit;
			_init = _atom->initialPosition();
		}

		_pos = _init;
		_pos.z = Z_DEF;
	}
	
	std::string display()
	{
		std::string str;
		hnet::Atom::Values val = _obj.value();

		switch (val)
		{
			case hnet::Atom::Oxygen:
			str = "O";
			break;

			case hnet::Atom::Nitrogen:
			str = "N";
			break;

			case hnet::Atom::Contradiction:
			str = "!";
			break;

			default:
			str = "?";
			break;
		}
		
		return str;
	}

	hnet::AtomConnector &_obj;
	Atom *_atom = nullptr;
};

class HydrogenProbe : public Probe
{
public:
	HydrogenProbe(hnet::HydrogenConnector &obj, 
	              AtomProbe &left, AtomProbe &right) :
	_obj(obj), _left(left), _right(right)
	{
		_init = left.position() + right.position();
		_init /= 2;
		_pos = _init;
	}
	
	std::string display()
	{
		std::string str;
		hnet::Hydrogen::Values val = _obj.value();

		switch (val)
		{
			case hnet::Hydrogen::Absent:
			str = " ";
			break;

			case hnet::Hydrogen::Present:
			str = "H";
			break;

			case hnet::Hydrogen::Contradiction:
			str = "!";
			break;

			default:
			str = "?";
			break;
		}
		
		return str;
	}

	hnet::HydrogenConnector &_obj;
	Atom *_atom = nullptr;
	AtomProbe &_left;
	AtomProbe &_right;
};

class BondProbe : public Probe
{
public:
	BondProbe(hnet::BondConnector &obj, Probe &left, Probe &right) :
	_obj(obj), _left(left), _right(right)
	{
		_init = left.position();
		_pos = _init;
	}

	virtual const glm::vec3 &position() const
	{
		return _left.position();
	}
	
	const glm::vec3 &end() const
	{
		return _right.position();
	}
	
	std::string display()
	{
		std::string str;
		hnet::Bond::Values val = _obj.value();

		switch (val)
		{
			case hnet::Bond::Weak:
			str = "weak_bond";
			break;

			case hnet::Bond::Strong:
			str = "strong_bond";
			break;

			case hnet::Bond::Absent:
			str = "transparency";
			break;

			case hnet::Bond::Present:
			str = "unassigned_bond";
			break;

			default:
			str = "unassigned_bond";
			break;
		}
		
		std::cout << "bond display: " << str << std::endl;
		
		return str;
	}

	hnet::BondConnector &_obj;
	Atom *_atom = nullptr;
	Probe &_left;
	Probe &_right;
};

#endif
