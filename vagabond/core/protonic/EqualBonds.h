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

#ifndef __vagabond__EqualBonds__
#define __vagabond__EqualBonds__

#include "hnet.h"

namespace hnet
{
struct EqualBonds
{
	EqualBonds(BondConnector &left, BondConnector &right)
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

			throw std::runtime_error("New equivalent bonds immediately "\
			                         "failed validation check");
		}
	}

	void forget(void *blame)
	{
		_left.forget(blame);
		_right.forget(blame);
	}

	bool check(void *prev)
	{
		Bond::Values forLeft = _left.value();
		Bond::Values forRight = _right.value();

		Bond::Values both = Bond::Values(forLeft & forRight);

		_left.assign_value(both, this, prev);
		if (is_contradictory(_left.value())) return false;

		_right.assign_value(both, this, prev);
		if (is_contradictory(_right.value())) return false;

		return true;
	}

	BondConnector &_left;
	BondConnector &_right;
};
};

#endif
