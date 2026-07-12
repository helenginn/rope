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

#ifndef __vagabond__Covalent2Count__
#define __vagabond__Covalent2Count__

#include "ConstraintBase.h"

namespace hnet
{
/* converts between covalent single/double bond & count */
struct Covalent2Count : public ConstraintBase
{
	Covalent2Count(CovalentConnector &cov, CountConnector &count)
	: _cov(cov), _count(count)
	{
		prep_constraints_and_forgets(this, {&cov, &count});
	}
	
	std::string desc()
	{
		return "covalent bond into count";
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);
		
		// first we check the covalent bond and what effect that has on count

		if (_cov.value() == Covalent::Unassigned)
		{
			assign(_count, Count::OneOrTwo, "single/double bond = 1/2");
		}
	else if (_cov.value() == Covalent::Double)
		{
			assign(_count, Count::Two, "double bond = 2");
		}
		else if (_cov.value() == Covalent::Single)
		{
			assign(_count, Count::One, "single bond = 1");
		}
		
		// next we check the count and what effect that has on covalent bond

		if (_count.value() == Count::One)
		{
			assign(_cov, Covalent::Single, "1 = single bond");
		}

		if (_count.value() == Count::Two)
		{
			assign(_cov, Covalent::Double, "2 = double bond");
		}

		return assign.okay();
	}

	CovalentConnector &_cov;
	CountConnector &_count;
};
};

#endif
