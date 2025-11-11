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

namespace hnet
{
/* simple constant class to impose a stricter belief when satisfying a
 * Condition */
template <class ImposeType, class CondType>
class Stricter
{
public:
	typedef std::function<bool(const CondType &)> Condition;

	Stricter(Connector<CondType> &obj, const Condition &cond, 
             Connector<ImposeType> &affected, ImposeType impose) :
	_condition(cond), _impose(impose), _conditionObj(obj), _imposeObj(affected)
	{
		std::vector<ConnectBase *> list = {&affected, &obj};
		prep_constraints_and_forgets(this, list);
	}
	
	std::string desc()
	{
		std::ostringstream ss;
		ss << "Stricter requirement on " << _conditionObj << " may impose "
		<< _impose << " on " << _imposeObj;
		return ss.str();
	}
	
	void forget(OpSet<void *> &blame)
	{
		_conditionObj.forget(blame);
		_imposeObj.forget(blame);
	}
	
	bool check(void *previous)
	{
		auto assign = make_assign_and_say(this, previous);
		if (_condition(_conditionObj.value()))
		{
//			std::cout << "Imposing " << _impose << " on " << _imposeObj << std::endl;
			assign(_imposeObj, _impose);
		}
		
		return assign.okay();
	}
	
	Condition _condition;
	ImposeType _impose;
	Connector<CondType> &_conditionObj;
	Connector<ImposeType> &_imposeObj;
};

class StricterBond : public Stricter<Bond::Values, Bond::Values>
{
public:
	StricterBond(Connector<Bond::Values> &bond, const Condition &cond, 
                 Bond::Values impose) :
	Stricter<Bond::Values, Bond::Values>(bond, cond, bond, impose) {};
};

typedef Stricter<Count::Values, Count::Values> IfCountThenImpose;
typedef Stricter<Bond::Values, Existence::Values> BreakIfUnsampledBond;
};


#endif

