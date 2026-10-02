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

#ifndef __vagabond__CountAdder__
#define __vagabond__CountAdder__

#include "ConstraintBase.h"

namespace hnet
{
/* simple adder in int form */
struct CountAdder : public ConstraintBase
{
	CountAdder(CountConnector &left, CountConnector &right, 
	           CountConnector &sum)
	: _left(left), _right(right), _sum(sum)
	{
		prep_constraints_and_forgets(this, {&left, &right, &sum});
	}
	
	std::string desc()
	{
		std::ostringstream ss;
		ss << "sum of " << _left << " and " << _right << " must be "
		<< _sum;
		return ss.str();
	}

	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);
		/* however, partial assignments of all values will lead to some
		 * values being permitted and others not, so we can generate a list
		 * of permitted values for each pair, and constrain the latter. */
		
		/* first we return to the comfortable world of integers... */
		CountList left_options = possible_values_list(_left.value());
		CountList right_options = possible_values_list(_right.value());

		/* impose options on sum */
		{
			Count::Values sum_should_be =
			combine_as_count(left_options, right_options,
			                 [](const int &a, const int &b) { return a + b; });

			// limited: options in existing sum which should be ruled out
			bool limit = (~sum_should_be & _sum.value());
			if (limit)
			{
				assign(_sum, sum_should_be, "imposition on sum");
			}
		}

		/* impose options on left */
		// right_options is still valid here - only _sum was assign()-ed
		// above, so re-deriving it from _right.value() would just repeat
		// the same call for the same answer.
		CountList sum_options = possible_values_list(_sum.value());

		{
			Count::Values left_should_be =
			combine_as_count(sum_options, right_options,
			                 [](const int &a, const int &b) { return a - b; });

			bool limit = (~left_should_be & _left.value());
			if (limit)
			{
				assign(_left, left_should_be, "imposition on left");
			}
		}

		/* impose options on right */
		sum_options = possible_values_list(_sum.value());
		left_options = possible_values_list(_left.value());

		{
			Count::Values right_should_be =
			combine_as_count(sum_options, left_options,
			                 [](const int &a, const int &b) { return a - b; });

			bool limit = (~right_should_be & _right.value());
			if (limit)
			{
				assign(_right, right_should_be, "imposition on right");
			}
		}

		return assign.okay();
	}

	CountConnector &_left;
	CountConnector &_right;
	CountConnector &_sum;
};
};

#endif
