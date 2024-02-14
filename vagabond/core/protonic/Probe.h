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

#define Z_DEF (-0)
#include <vagabond/core/Atom.h>
#include "Connector.h"
#include <vagabond/core/Responder.h>

class Probe : public HasResponder<Responder<Probe>>
{
public:
	virtual const glm::vec3 &position() const
	{
		return _pos;
	}
	
	virtual ~Probe()
	{

	}
	
	const glm::vec3 colour() const
	{
		return _colour;
	}
	
	void setColour(const glm::vec3 &colour)
	{
		_colour = colour;
	}

	void setPosition(const glm::vec2 &position)
	{
		_pos = glm::vec3(position, Z_DEF);
	}
	
	void register_probe(Probe *other)
	{
		_others.push_back(other);
	}
	
	std::vector<Probe *> &others()
	{
		return _others;
	}
	
	virtual bool is_text() = 0;
	virtual std::string display() = 0;

	virtual bool is_atom()
	{
		return false;
	}
	
	virtual bool is_certain() = 0;
	
	void setAlpha(float alpha)
	{
		_alpha = alpha;
		
		for (Probe *other : _others)
		{
			other->setAlpha(_alpha);
		}
		sendResponse("alpha", this);
	}

	float alpha() const
	{
		return _alpha;
	}

	void setMult(const float &m)
	{
		_mult = m;
	}
	
	float mult()
	{
		return _mult;
	}
	
	::Atom *const &atom() const
	{
		return _atom;
	}
	
	Atom *_atom = nullptr;
	glm::vec3 _pos = {};
	glm::vec3 _init = {};
	
	std::vector<Probe *> _others;
	glm::vec3 _colour = {};
	float _mult = 25;
	float _alpha = 0.f;
};

class AtomProbe : public Probe
{
public:
	AtomProbe(hnet::AtomConnector &obj, Atom *inherit = nullptr,
	          const std::string &custom_text = {}) : _obj(obj)
	{
		if (inherit)
		{
			_atom = inherit;
			_init = _atom->initialPosition();
			_colour = (_atom->code() == "HOH" ? 
			           glm::vec3(0, 0, 0.0) : glm::vec3(-0.3f));
		}

		_pos = _init;
		_text = custom_text;
	}

	virtual bool is_text()
	{
		return true;
	}
	
	virtual bool is_atom()
	{
		return true;
	}
	
	virtual std::string display()
	{
		if (_text.length())
		{
			return _text;
		}

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

			case hnet::Atom::Sulphur:
			str = "S";
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

	virtual bool is_certain()
	{
		bool good = true;
		for (Probe *const &other : others())
		{
			if (!other->is_certain())
			{
				good = false;
			}
		}
		return _obj.is_certain() && good;
	}

	std::string _text;
	hnet::AtomConnector &_obj;
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

		left.register_probe(this);
		right.register_probe(this);
	}
	
	virtual std::string display()
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

			case hnet::Hydrogen::Unassigned:
			str = "?";
			break;

			default:
			str = "?";
			break;
		}
		
		return str;
	}

	virtual bool is_text()
	{
		return true;
	}

	const AtomProbe &left() const
	{
		return _left;
	}

	const AtomProbe &right() const
	{
		return _right;
	}

	virtual bool is_certain()
	{
		return _obj.is_certain();
	}

	hnet::HydrogenConnector &_obj;
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
		
		left.register_probe(this);
		right.register_probe(this);
	}

	virtual const glm::vec3 &position() const
	{
		return _left.position();
	}
	
	const glm::vec3 &end() const
	{
		return _right.position();
	}

	virtual bool is_text()
	{
		return false;
	}
	
	virtual std::string display()
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

			case hnet::Bond::Broken:
			str = "transparency";
			break;

			case hnet::Bond::NotPresent:
			str = "transparency";
			break;

			case hnet::Bond::Present:
			str = "present_bond";
			break;

			default:
			str = "unassigned_bond";
			break;
		}
		
		return str;
	}

	virtual bool is_certain()
	{
		return _obj.is_certain();
	}

	hnet::BondConnector &_obj;
	Probe &_left;
	Probe &_right;
};

class CountProbe : public Probe
{
public:
	CountProbe(hnet::CountConnector &obj, Atom *atom) :
	_obj(obj)
	{
		_pos = atom->initialPosition();
		_pos = _init;
		_atom = atom;
	}

	virtual bool is_text()
	{
		return false;
	}
	
	virtual std::string display()
	{
		std::string str;
		hnet::Count::Values val = _obj.value();
		std::vector<int> options = hnet::possible_values(val);

		std::string list;
		for (const int &opt : options)
		{
			list += std::to_string(opt) + "/";
		}
		list.pop_back();
		
		return list;
	}

	virtual bool is_certain()
	{
		return _obj.is_certain();
	}

	hnet::CountConnector &_obj;
};

#endif
