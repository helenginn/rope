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
		auto self_check = [this](void *prev) { return check(prev); };

		_left.add_constraint_check(self_check);
		_right.add_constraint_check(self_check);
		
		auto forget_me = [this](void *blame) { return forget(blame); };

		_left.add_forget(forget_me);
		_right.add_forget(forget_me);

		if (!check(this))
		{
			_left.pop_last_check(this);
			_right.pop_last_check(this);

			throw std::runtime_error("New either/or dichotomy immediately "\
			                         "failed validation check");
		}
	}
	
	void forget(void *blame)
	{
		_left.forget(blame);
		_right.forget(blame);
	}

	bool check(void *previous)
	{
		if (_left.value() == Bond::Unassigned)
		{
			_right.assign_value(Bond::Unassigned, this, previous);
		}
		if (_right.value() == Bond::Unassigned)
		{
			_left.assign_value(Bond::Unassigned, this, previous);
		}

		if ((_left.value() & Bond::Present) && 
		    !(_left.value() & Bond::Absent))
		{
			_right.assign_value(Bond::Absent, this, previous);
		}
		else if ((_right.value() & Bond::Present) &&
		         !(_right.value() & Bond::Absent))
		{
			_left.assign_value(Bond::Absent, this, previous);
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
