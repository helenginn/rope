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
#include <string>
#include <vagabond/core/Atom.h>
#include <vagabond/core/Responder.h>

namespace hnet
{
	typedef std::function<float()> GetEnergy;
	typedef std::function<GetEnergy()> EnergyWrapper;
};

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
	
	void setBulk(const bool &bulk)
	{
		_bulk = bulk;
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

	virtual bool is_bulk()
	{
		return _bulk;
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

	// the probe's current resolved value as a plain int, for recording
	// into a ProbeResult/OneProbe. Meaningful once is_certain() is true;
	// callers should check that first. `type` matters for BondProbe,
	// which is tracked as two separate ProbeTypePair slots (its own
	// existence, and - once present - which kind of bond); other probe
	// types only ever have one meaningful value and ignore it.
	virtual int certainValueAsInt() = 0;
	
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
		return std::max(-1.f, _hide + transparency());
	}

	void setMult(const float &m)
	{
		_mult = m;
	}
	
	float mult()
	{
		return _mult;
	}

	void setEnergyWrapper(const hnet::EnergyWrapper &f)
	{
		_energy = f;
	}
	
	hnet::GetEnergy energy()
	{
		if (!_energy)
		{
			return {};
		}
		return _energy();
	}

	void realign()
	{
		if (_realign) { _realign(); };
	}
	
	::Atom *const &atom() const
	{
		return _atom;
	}
	
	virtual hnet::AtomConf atomConf() const
	{
		return {_atom, _conf};
	}
	
	Atom *_atom = nullptr;
	Atom *_h = nullptr;
	char _conf;
	glm::vec3 _pos = {};
	glm::vec3 _init = {};
	glm::vec4 _glow = {};
	
	std::vector<Probe *> _others;
	glm::vec3 _colour = {};
	float _mult = 25;
	float _hide = 0.f;
	bool _bulk = false;
	hnet::EnergyWrapper _energy;

	std::function<void()> _realign{};
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
		hnet::Atom::Values val = _obj.value(true);

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

			case hnet::Atom::Ion:
			str = "M";
			break;

			case hnet::Atom::Contradiction:
			str = "!";
			break;

			default:
			str = "?";
			break;
		}
		
		if (str == "M" && _atom)
		{
			str = _atom->elementSymbol();
			if (str.length() > 1)
			{
				str[1] = std::tolower(str[1]);
			}
		}
		
		return str;
	}

	virtual float transparency()
	{
		if (!_exist.is_certain(true))
		{
			return -0.5f;
		}
		else if (_exist.value(true) == hnet::Existence::Absent)
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

	virtual int certainValueAsInt()
	{
		return (int)_exist.value();
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
	              hnet::ExistenceConnector &exist, Atom *hAtom,
	              AtomProbe *left = {}, AtomProbe *right = {}) :
	_obj(obj), _exist(exist), _left(left), _right(right)
	{
		_init = hAtom->initialPosition();
		_pos = _init;
		_h = hAtom;
		_conf = hAtom->conformerPositions().begin()->first[0];

		_colour = glm::vec3(0.28f, 0.1f, -0.147f);
	}
	
	virtual hnet::AtomConf atomConf() const
	{
		return {_h, _conf};
	}
	
	virtual std::string display()
	{
		if (!_right) return "";
		std::string str;
		hnet::Existence::Values val = _obj.value(true);

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

	void setAtomPosition(const glm::vec3 &pos)
	{
		_h->setInitialPosition(pos);
	}
	
	void setPosition(const glm::vec3 &pos)
	{
		_pos = pos;
		_h->setInitialPosition(pos);
		_obj._update(true);
	}

	virtual bool is_text()
	{
		return true;
	}

	virtual std::string desc()
	{
		return _obj.desc();
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
		if (!_exist.is_certain(true))
		{
			return -0.5f;
		}
		else if (_exist.is_certain(true) &&
		         _exist.value(true) == hnet::Existence::Absent)
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

	virtual int certainValueAsInt()
	{
		return (int)_obj.value();
	}

	virtual bool is_absent()
	{
		return _obj.value() == hnet::Existence::Absent;
	}

	hnet::ExistenceConnector &_obj;
	hnet::ExistenceConnector &_exist;
	AtomProbe *_left{};
	AtomProbe *_right{};
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
		return !(_obj.value() & hnet::Bond::Bonded);
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
		if (!_exist.is_certain(true))
		{
			return -0.5f;
		}
		else if (_exist.is_certain(true) &&
		         _exist.value(true) == hnet::Existence::Absent)
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
		hnet::Bond::Values val = _obj.value(true);

		switch (val)
		{
			case hnet::Bond::Weak:
			str = "weak_bond";
			break;

			case hnet::Bond::Strong:
			str = "strong_bond";
			break;

			case hnet::Bond::LonePair:
			str = "lone_pair";
			break;

			case hnet::Bond::Broken:
			str = "transparency";
			break;

			case hnet::Bond::NotBonded:
			str = "transparency";
			break;

			case hnet::Bond::Bonded:
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

	virtual int certainValueAsInt()
	{
		// same collapse for both slots: absent existence or a
		// functionally-absent bond state (broken/lone pair) means there
		// is nothing more to report either way.
		if (_exist.value() == hnet::Existence::Absent)
		{
			return (int)hnet::Existence::Absent;
		}
		if (_obj.value() == hnet::Bond::Broken ||
		    _obj.value() == hnet::Bond::LonePair)
		{
			return (int)hnet::Existence::Absent;
		}

		return (int)_obj.value();
	}

	hnet::ExistenceConnector &existence()
	{
		return _exist;
	}

	hnet::BondConnector &_obj;
	Probe &_left;
	Probe &_right;
	hnet::ExistenceConnector &_exist;
	float _distance{};
};


#endif
