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

namespace hnet
{
/* simple adder in int form */
struct CountAdder
{
	CountAdder(CountConnector &left, CountConnector &right, 
	           CountConnector &sum)
	: _left(left), _right(right), _sum(sum)
	{
		auto self_check = [this](void *prev) { return check(prev); };
		
		_sum.add_constraint_check(self_check);
		_left.add_constraint_check(self_check);
		_right.add_constraint_check(self_check);

		auto forget_me = [this](void *blame) { return forget(blame); };

		_left.add_forget(forget_me);
		_sum.add_forget(forget_me);
		_right.add_forget(forget_me);

		if (!check(this))
		{
			_left.pop_last_check(this);
			_sum.pop_last_check(this);
			_right.pop_last_check(this);

			throw std::runtime_error("New addition of Adder immediately "\
			                         "failed validation check");
		}

	}
	
	void forget(void *blame)
	{
		_left.forget(blame);
		_sum.forget(blame);
		_right.forget(blame);
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
	
	bool check(void *previous)
	{
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
			

			Count::Values sum_count = values_as_count(options);
			_sum.assign_value(sum_count, this, previous);
		}

		/* impose options on left */
		std::vector<int> sum_options = possible_values(_sum.value());
		right_options = possible_values(_right.value());

		{
			std::vector<int> options = permutations(sum_options, right_options,
			                                        [](const int &a, const int &b)
			                                        { return a - b; });

			Count::Values left_count = values_as_count(options);
			_left.assign_value(left_count, this, previous);
		}

		/* impose options on right */
		sum_options = possible_values(_sum.value());
		left_options = possible_values(_left.value());

		{
			std::vector<int> options = permutations(sum_options, left_options,
			                                        [](const int &a, const int &b)
			                                        { return a - b; });

			Count::Values right_count = values_as_count(options);
			_right.assign_value(right_count, this, previous); 
		}

		return (!is_contradictory(_sum.value()) || 
		        !is_contradictory(_left.value()) ||
		        !is_contradictory(_right.value()));
	}

	CountConnector &_left;
	CountConnector &_right;
	CountConnector &_sum;
};
};

#endif
