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

#ifndef __vagabond__EitherOrBond__
#define __vagabond__EitherOrBond__

#include "hnet.h"
#include "ConstraintBase.h"

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct EitherOrBond : public ConstraintBase
{
	EitherOrBond(BondConnector &left, BondConnector &right, bool break_only)
	: _left(left), _right(right)
	{
		prep_constraints_and_forgets(this, {&left, &right});
		_breakOnly = break_only;
	}

	std::string desc()
	{
		std::ostringstream ss;
		ss << "either " << _left << " or " << _right << " may be present, "\
		"not both";
		return ss.str();
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		if ((_left.value() & Bond::NotBroken) && 
		    !(_left.value() & Bond::Broken))
		{
			assign(_right, Bond::Broken);
		}

		return assign.okay();
	}

	BondConnector &_left;
	BondConnector &_right;
	bool _breakOnly{};
};
};

#endif
