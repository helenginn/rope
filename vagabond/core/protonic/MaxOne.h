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


#ifndef __vagabond__MaxOne__
#define __vagabond__MaxOne__

#include "hnet.h"
#include "ConstraintBase.h"

namespace hnet
{
struct MaxOne : public ConstraintBase
{
public:
	MaxOne(ExistenceConnector &left, ExistenceConnector &right)
	: _left(left), _right(right)
	{
		prep_constraints_and_forgets(this, {&left, &right});
	}

	std::string desc()
	{
		return "clash prevention between \"" + _left.desc() + 
		"\" and \"" + _right.desc() + "\"";
	}

	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		Existence::Values forLeft = _left.value();
		Existence::Values forRight = _right.value();

		if (forLeft == Existence::Present)
		{
			assign(_right, Existence::Absent);
		}
		else if (forRight == Existence::Present)
		{
			assign(_left, Existence::Absent);
		}
		
		return assign.okay();
	}
private:
	ExistenceConnector &_left;
	ExistenceConnector &_right;

};
};

#endif
