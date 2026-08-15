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

#ifndef __vagabond__CountProbe__
#define __vagabond__CountProbe__

#include "Probe.h"

class CountProbe : public Probe
{
public:
	CountProbe(hnet::CountConnector &obj, hnet::ExistenceConnector &exist,
           hnet::AtomConf ac) :
	_obj(obj), _exist(exist)
	{
		_pos = ac.position();
//		_pos = _init;
		_atom = ac.ptr;
		_conf = ac.conf;
		_offset = 2;
	}

	CountProbe(hnet::CountConnector &obj, hnet::ExistenceConnector &exist,
           std::vector<hnet::AtomConf> acs, float offset) :
	_obj(obj), _exist(exist), _offset(offset)
	{
		_pos = {};
		for (const hnet::AtomConf &ac : acs)
		{
			_pos += ac.position();
		}
		_pos /= (float)acs.size();
		_atom = acs.front().ptr;
		_conf = acs.front().conf;
	}

	virtual bool is_text()
	{
		return false;
	}

	virtual bool is_charge()
	{
		return true;
	}

	virtual float transparency()
	{
		float val = 0;
		if (!_exist.is_certain(true))
		{
			val = -0.5f;
		}
		else if (_exist.value(true) == hnet::Existence::Absent)
		{
			val = -1.0f;
		}
		else 
		{
			val = -0.0f;
		}
		return val + _hide;
	}
	
	virtual std::string display()
	{
		std::string str;
		hnet::Count::Values val = _obj.value(true);

		std::vector<int> options = hnet::possible_values(val);
		if (options.size() > 1)
		{
			return "circle_question";
		}
		if (options.size() == 0 || options[0] == 0)
		{
			return "transparency";
		}
		
		switch (options[0])
		{
			case 1:
			return "circle_plus";
			case 2:
			return "circle_2plus";
			case 3:
			return "circle_3plus";
			case -1:
			return "circle_minus";
			case -2:
			return "circle_2minus";
			case -3:
			return "circle_3minus";
			default:
			return "transparency";
		}
		return "transparency";
	}

	virtual std::string desc()
	{
		return _obj.desc();
	}

	virtual bool is_certain()
	{
		return _obj.is_certain();
	}

	virtual int certainValueAsInt()
	{
		if (_exist.value() == hnet::Existence::Absent)
		{
			return (int)_exist.value();
		}
		return (int)_obj.value();
	}

	hnet::CountConnector &_obj;
	hnet::ExistenceConnector &_exist;
	float _offset = 2.f;
};


#endif
