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

#ifndef __vagabond__PeggedExistence__
#define __vagabond__PeggedExistence__

#include "hnet.h"
#include "ConstraintBase.h"

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct PeggedExistence : public ConstraintBase
{
	PeggedExistence(ExistenceConnector &left, ExistenceConnector &sub,
	             ExistenceConnector &right)
	: _left(left), _sub(sub), _right(right)
	{
		prep_constraints_and_forgets(this, {&left, &sub, &right});
	}

	std::string desc()
	{
		std::ostringstream ss;
		ss << "subservient existence of " << _sub <<
		" dependent on " << _left << " and " << _right;
		return ss.str();

	}

	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		if (_left.value() == Existence::Present &&
		    _right.value() == Existence::Present)
		{
			assign(_sub, Existence::Present);
		}

		if (_left.value() == Existence::Absent &&
		    _right.value() == Existence::Absent)
		{
			assign(_sub, Existence::Absent, "both left and right existences"\
			       " were absent so middle existence should also be absent");
		}

		if (_sub.value() == Existence::Absent)
		{
			if (_left.value() & Existence::Absent)
			{
				assign(_left, Existence::Absent, "middle existence was absent"\
				" and left existence could be absent");
			}
			if (_right.value() & Existence::Absent)
			{
				assign(_right, Existence::Absent, "middle existence was absent"\
				       "and right existence could be absent");
			}
		}

		return assign.okay();
	}

	ExistenceConnector &_left;
	ExistenceConnector &_sub;
	ExistenceConnector &_right;
};
};

#endif
