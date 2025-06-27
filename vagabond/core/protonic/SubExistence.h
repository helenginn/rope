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

#ifndef __vagabond__SubExistence__
#define __vagabond__SubExistence__

#include "hnet.h"

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct SubExistence
{
	SubExistence(ExistenceConnector &left, ExistenceConnector &sub,
	             ExistenceConnector &right)
	: _left(left), _sub(sub), _right(right)
	{
		auto self_check = [this](void *prev) { return check(prev); };

		_left.add_constraint_check(self_check);
		_sub.add_constraint_check(self_check);
		_right.add_constraint_check(self_check);
		
		auto forget_me = [this](void *blame) { return forget(blame); };

		_left.add_forget(forget_me);
		_sub.add_forget(forget_me);
		_right.add_forget(forget_me);

		if (!check(this))
		{
			_left.pop_last_check(this);
			_sub.pop_last_check(this);
			_right.pop_last_check(this);

			throw std::runtime_error("New mutual existence immediately "\
			                         "failed validation check");
		}
	}
	
	std::string desc()
	{
		std::ostringstream ss;
		ss << "subservient existence of " << _sub << 
		" dependent on " << _left << " and " << _right;
		return ss.str();

	}
	
	void forget(void *blame)
	{
		_left.forget(blame);
		_sub.forget(blame);
		_right.forget(blame);
	}

	bool check(void *previous)
	{
		auto assign = make_assign_and_say(this, previous);

		if (_left.value() == Existence::Present && 
		    _right.value() == Existence::Present)
		{
			assign(_sub, Existence::Present);
		}

		if (_left.value() == Existence::Absent ||
		    _right.value() == Existence::Absent)
		{
			assign(_sub, Existence::Absent);
		}

		bool con = (!is_contradictory(_left.value()) &&
		            !is_contradictory(_right.value()));
		
		return con;
	}

	ExistenceConnector &_left;
	ExistenceConnector &_sub;
	ExistenceConnector &_right;
};
};

#endif
