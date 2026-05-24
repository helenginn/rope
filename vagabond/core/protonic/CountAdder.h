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

	
	template <class Op>
	std::vector<int> permutations(const std::vector<int> &vs, 
	                              const std::vector<int> &ws,
	                              const Op &op)
	{
		std::vector<int> options;
		options.reserve(vs.size() * ws.size());

		for (const int &v : vs)
		{
			for (const int &w : ws)
			{
				options.push_back(op(v, w));
			}
		}
		return options;
	}
	
	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);
		/* however, partial assignments of all values will lead to some
		 * values being permitted and others not, so we can generate a list
		 * of permitted values for each pair, and constrain the latter. */
		
		/* first we return to the comfortable world of integers... */
		std::vector<int> left_options = possible_values(_left.value());
		std::vector<int> right_options = possible_values(_right.value());

		/* impose options on sum */
		{
			std::vector<int> options = permutations(left_options, right_options,
			                                        [](const int &a, const int &b)
			                                        { return a + b; });

			Count::Values sum_should_be = values_as_count(options);

			// limited: options in existing sum which should be ruled out
			bool limit = (~sum_should_be & _sum.value());
//			if (limit)
			{
				assign(_sum, sum_should_be, "imposition on sum");
			}
		}

		/* impose options on left */
		std::vector<int> sum_options = possible_values(_sum.value());
		right_options = possible_values(_right.value());

		{
			std::vector<int> options = permutations(sum_options, right_options,
			                                        [](const int &a, const int &b)
			                                        { return a - b; });

			Count::Values left_should_be = values_as_count(options);
			bool limit = (~left_should_be & _left.value());
//			if (limit)
			{
				assign(_left, left_should_be, "imposition on left");
			}
		}

		/* impose options on right */
		sum_options = possible_values(_sum.value());
		left_options = possible_values(_left.value());

		{
			std::vector<int> options = permutations(sum_options, left_options,
			                                        [](const int &a, const int &b)
			                                        { return a - b; });

			Count::Values right_should_be = values_as_count(options);
			bool limit = (~right_should_be & _right.value());
//			if (limit)
			{
				assign(_right, right_should_be, "imposition on right");
			}
		}
		
		auto print = [](const std::vector<int> &options)
		{
			for (const int &i : options)
			{
				std::cout << i << " ";
			}
			std::cout << std::endl;
		};

		return assign.okay();
	}

	CountConnector &_left;
	CountConnector &_right;
	CountConnector &_sum;
};
};

#endif
