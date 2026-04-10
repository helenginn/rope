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

#ifndef __vagabond__Stricter__
#define __vagabond__Stricter__

#include "hnet.h"
#include "Connector.h"
#include "ConstraintBase.h"

namespace hnet
{
/* simple constant class to impose a stricter belief when satisfying a
 * Condition */
template <class ImposeType>
class Stricter : public ConstraintBase
{
public:
	typedef std::function<bool()> Condition;

	Stricter(const std::vector<ConnectBase *> &objs, 
	         const Condition &cond, Connector<ImposeType> &affected, 
         ImposeType impose) :
	_condition(cond), _impose(impose), _conditionObjs(objs), 
	_imposeObj(affected)
	{
		std::vector<ConnectBase *> list = objs;
		list.push_back({&affected});
		prep_constraints_and_forgets(this, list);
	}
	
	std::string desc()
	{
		std::ostringstream ss;
		ss << "Stricter requirement on ";
		for (auto obj : _conditionObjs)
		{
			ss << obj << ", ";
		}
		
		ss << " may impose " << _impose << " on " << _imposeObj;
		return ss.str();
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);
		if (_condition())
		{
//			std::cout << "Imposing " << _impose << " on " << _imposeObj << std::endl;
			assign(_imposeObj, _impose);
		}
		
		return assign.okay();
	}
	
	Condition _condition;
	ImposeType _impose;
	std::vector<ConnectBase *> _conditionObjs;
	Connector<ImposeType> &_imposeObj;
};

/*
class StricterBond : public Stricter<Bond::Values>
{
public:
	StricterBond(Connector<Bond::Values> &bond, const Condition &cond, 
                 Bond::Values impose) :
	Stricter<Bond::Values>({&bond}, cond, bond, impose) {};
};
*/

typedef Stricter<Bond::Values> StricterBond;
typedef Stricter<Existence::Values> StrictExistence;
typedef Stricter<Count::Values> StrictCount;
};


#endif

