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

#ifndef __vagabond__MutualExistence__
#define __vagabond__MutualExistence__

#include "hnet.h"

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct MutualExistence
{
	MutualExistence(ExistenceConnector &left, ExistenceConnector &right,
	                std::string desc = "")
	: _left(left), _right(right), _desc(desc)
	{
		prep_constraints_and_forgets(this, {&left, &right});
	}
	
	void forget(OpSet<void *> &blame)
	{
		_left.forget(blame);
		_right.forget(blame);
	}
	
	std::string desc()
	{
		std::ostringstream ss;
		ss << "mutual existence between " << _left << " and " << _right;
		return ss.str();
	}

	bool check(void *previous)
	{
		auto assign = make_assign_and_say(this, previous);

		if (_left.value() == Existence::Present)
		{
			assign(_right, Existence::Present);
		}
		else if (_right.value() == Existence::Present)
		{
			assign(_left, Existence::Present);
		}
		else if (_left.value() == Existence::Absent)
		{
			assign(_right, Existence::Absent);
		}
		else if (_right.value() == Existence::Absent)
		{
			assign(_left, Existence::Absent);
		}

		return assign.okay();
	}

	ExistenceConnector &_left;
	ExistenceConnector &_right;
	std::string _desc{};
};
};

#endif
