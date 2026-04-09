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
	                bool strong = true, std::string desc = "")
	: _left(left), _right(right), _desc(desc), _strong(strong)
	{
		prep_constraints_and_forgets(this, {&left, &right});
	}
	
	void forget(const GuiltVersion &gv)
	{
		_left.forget(gv);
		_right.forget(gv);
	}
	
	std::string desc()
	{
		std::ostringstream ss;
		ss << "mutual existence between " << _left << " and " << _right;
		return ss.str();
	}

	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		if (_left.value() == Existence::Present && _strong)
		{
			assign(_right, Existence::Present);
		}
		else if (_left.value() == Existence::Absent)
		{
			assign(_right, Existence::Absent);
		}

		return assign.okay();
	}

	ExistenceConnector &_left;
	ExistenceConnector &_right;
	std::string _desc{};
	bool _strong{};
};
};

#endif
