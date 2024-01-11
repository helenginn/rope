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

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct EitherOrBond
{
	EitherOrBond(BondConnector &left, BondConnector &right)
	: _left(left), _right(right)
	{
		auto self_check = [this]() { return check(); };

		_left.add_constraint_check(self_check);
		_right.add_constraint_check(self_check);

		if (!check())
		{
			_left.pop_last_check();
			_right.pop_last_check();

			throw std::runtime_error("New either/or dichotomy immediately "\
			                         "failed validation check");
		}
	}

	bool check()
	{
		Bond::Values forLeft = _left.value();
		Bond::Values forRight = _right.value();
		
		if ((_left.value() & Bond::Present) && 
		    !(_left.value() & Bond::Absent))
		{
			_right.assign_value(Bond::Absent, this);
		}
		else if ((_right.value() & Bond::Present) &&
		         !(_right.value() & Bond::Absent))
		{
			_left.assign_value(Bond::Absent, this);
		}

		bool con = (!is_contradictory(_left.value()) &&
		            !is_contradictory(_right.value()));
		
		return con;
	}

	BondConnector &_left;
	BondConnector &_right;
};
};

#endif
