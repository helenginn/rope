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
#include "Connector.h"
#include <vagabond/core/Atom.h>
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
	
	const glm::vec4 glow() const
	{
		return _glow;
	}
	
	void setColour(const glm::vec3 &colour)
	{
		_colour = colour;
	}

	void setPosition(const glm::vec3 &position)
	{
		_pos = position;
	}

	virtual bool is_absent()
	{
		return false;
	}

	virtual bool is_covalent()
	{
		return false;
	}

	virtual bool is_bond()
	{
		return false;
	}

	virtual bool is_definitely_not_present()
	{
		return false;
	}
	
	virtual std::string desc() = 0;
	
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
	virtual std::string value()
	{
		return "";
	}

	virtual bool is_atom()
	{
		return false;
	}
	
	virtual bool is_certain() = 0;
	
	void setHide(float alpha, OpSet<Probe *> &fixed)
	{
		_hide = alpha;
		fixed.insert(this);
		
		for (Probe *other : _others)
		{
			if (fixed.count(other) == 0)
			{
				other->setHide(_hide, fixed);
			}
		}
		sendResponse("alpha", this);
	}

	void setHide(float alpha, bool recursive = true)
	{
		if (recursive)
		{
			OpSet<Probe *> fixed;
			setHide(alpha, fixed);
		}
		else
		{
			_hide = alpha;
			sendResponse("alpha", this);
		}
	}

	virtual float transparency()
	{
		return 0.f;
	}

	float alpha()
	{
		return _hide + transparency();
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
	
	hnet::AtomConf atomConf() const
	{
		return {_atom, _conf};
	}
	
	Atom *_atom = nullptr;
	char _conf;
	glm::vec3 _pos = {};
	glm::vec3 _init = {};
	glm::vec4 _glow = {};
	
	std::vector<Probe *> _others;
	glm::vec3 _colour = {};
	float _mult = 25;
	float _hide = 0.f;
};

class AtomProbe : public Probe
{
public:
	AtomProbe(hnet::AtomConnector &obj, hnet::ExistenceConnector &exist,
	          Atom *inherit = nullptr, char conf = '\0', 
	          const std::string &custom_text = {})
	: _obj(obj), _exist(exist)
	{
		if (inherit)
		{
			_atom = inherit;
			_conf = conf;
			hnet::AtomConf ac = {_atom, _conf};
			_init = ac.position();
			_colour = glm::vec3(-0.3f);
			if (_atom->code() == "HOH")
			{
				_colour = glm::vec3(0.6, -0.2, -0.2);
				_glow = {0.3f, 0.3f, 2.f, 1.f};
			}
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
	
	virtual std::string desc()
	{
		return atomConf().desc();
	}

	virtual std::string display()
	{
		if (_text.length())
		{
			return _text;
		}

		std::string str;
		bool accessed = false;
		hnet::Atom::Values val = _obj.value(&accessed);
		if (!accessed)
		{
			return "";
		}

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

			case hnet::Atom::Inactive:
			str = " ";
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

	virtual float transparency()
	{
		if (!_exist.is_certain())
		{
			return -0.5f;
		}
		else if (_exist.value() == hnet::Existence::Absent)
		{
			return -1.0f;
		}
		else 
		{
			return -0.0f;
		}
	}

	virtual bool is_certain()
	{
		return _obj.is_certain() && _exist.is_certain();
	}
	
	hnet::ExistenceConnector &existence()
	{
		return _exist;
	}

	std::string _text;
	hnet::AtomConnector &_obj;
	hnet::ExistenceConnector &_exist;
};

class HydrogenProbe : public Probe
{
public:
	HydrogenProbe(hnet::ExistenceConnector &obj, 
	              hnet::ExistenceConnector &exist,
	              AtomProbe &left, AtomProbe &right) :
	_obj(obj), _exist(exist), _left(left), _right(right)
	{
		_init = left.position() + right.position();
		_init /= 2;
		_pos = _init;

		left.register_probe(this);
		right.register_probe(this);

		register_probe(&left);
		register_probe(&right);
		_colour = glm::vec3(0.28f, 0.1f, -0.147f);
	}
	
	virtual std::string display()
	{
		std::string str;
		bool accessed = false;
		hnet::Existence::Values val = _obj.value(&accessed);
		if (!accessed)
		{
			return "";
		}

		switch (val)
		{
			case hnet::Existence::Absent:
			str = " ";
			break;

			case hnet::Existence::Present:
			str = "H";
			break;

			case hnet::Existence::Contradiction:
			str = "!";
			break;

			case hnet::Existence::Unassigned:
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

	virtual std::string desc()
	{
		return _obj.desc();
	}

	const AtomProbe &left() const
	{
		return _left;
	}

	const AtomProbe &right() const
	{
		return _right;
	}

	hnet::ExistenceConnector &existence()
	{
		return _exist;
	}

	virtual bool is_definitely_not_present()
	{
		return (_obj.value() == hnet::Existence::Absent);
	}

	virtual float transparency()
	{
		if (!_exist.is_certain())
		{
			return -0.5f;
		}
		else if (_exist.is_certain() &&
		         _exist.value() == hnet::Existence::Absent)
		{
			return -1.0f;
		}
		else 
		{
			return -0.0f;
		}
	}


	virtual bool is_certain()
	{
		return _obj.is_certain() && _exist.is_certain();
	}

	virtual bool is_absent()
	{
		return _obj.value() == hnet::Existence::Absent;
	}

	hnet::ExistenceConnector &_obj;
	hnet::ExistenceConnector &_exist;
	AtomProbe &_left;
	AtomProbe &_right;
};


class BondProbe : public Probe
{
public:
	BondProbe(hnet::BondConnector &obj, Probe &left, Probe &right,
          hnet::ExistenceConnector &exist) :
	_obj(obj), _left(left), _right(right), _exist(exist)
	{
		_init = left.position();
		_pos = _init;
		
		left.register_probe(this);
		right.register_probe(this);
		
		register_probe(&left);
		register_probe(&right);
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

	Probe &left() const
	{
		return _left;
	}

	Probe &right() const
	{
		return _right;
	}

	virtual std::string desc()
	{
		return _obj.desc();
	}

	virtual bool is_definitely_not_present()
	{
		return !(_obj.value() & hnet::Bond::Present);
	}

	virtual std::string value()
	{
		std::ostringstream ss;
		ss << "{" << _obj.value() << "} and ";
		ss << "{" << _exist.value() << "}";
		return ss.str();
	}

	virtual float transparency()
	{
		if (!_exist.is_certain())
		{
			return -0.5f;
		}
		else if (_exist.is_certain() &&
		         _exist.value() == hnet::Existence::Absent)
		{
			return -1.0f;
		}
		else 
		{
			return -0.0f;
		}
	}


	virtual bool is_bond()
	{
		return true;
	}

	virtual std::string display()
	{
		std::string str;
		bool accessed = false;
		hnet::Bond::Values val = _obj.value(&accessed);
		if (!accessed)
		{
			return "";
		}

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
		if (_exist.value() == hnet::Existence::Absent)
		{
			return true;
		}

		return _obj.is_certain() && _exist.is_certain();
	}

	hnet::ExistenceConnector &existence()
	{
		return _exist;
	}

	hnet::BondConnector &_obj;
	Probe &_left;
	Probe &_right;
	hnet::ExistenceConnector &_exist;
};

class CovalentProbe : public BondProbe
{
public:
	CovalentProbe(Probe &left, Probe &right, hnet::ExistenceConnector &exist, 
	              bool doubleBond) 
	: BondProbe(_silent, left, right, exist)
	{
		_doubleBond = doubleBond;
	}

	virtual const glm::vec3 &position() const
	{
		return _left.position();
	}
	
	const glm::vec3 &end() const
	{
		return _right.position();
	}

	virtual bool is_covalent()
	{
		return true;
	}

	virtual bool is_text()
	{
		return false;
	}

	virtual std::string desc()
	{
		return "covalent bond between " + _left.desc() + " and " + _right.desc();
	}
	
	virtual std::string display()
	{
		if (_doubleBond)
		{
			return "double_bond";
		}
		else
		{
			return "single_bond";
		}
	}

	bool _doubleBond{false};
	hnet::BondConnector _silent{};
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
		bool accessed = false;
		hnet::Count::Values val = _obj.value(&accessed);
		if (!accessed)
		{
			return "";
		}
		std::vector<int> options = hnet::possible_values(val);

		std::string list;
		for (const int &opt : options)
		{
			list += std::to_string(opt) + "/";
		}
		list.pop_back();
		
		return list;
	}

	virtual std::string desc()
	{
		return _obj.desc();
	}

	virtual bool is_certain()
	{
		return _obj.is_certain();
	}

	hnet::CountConnector &_obj;
};

#endif
