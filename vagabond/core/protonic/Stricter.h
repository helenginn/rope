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

#ifndef __vagabond__Stricter__
#define __vagabond__Stricter__

#include "hnet.h"

namespace hnet
{
/* simple constant class to impose a stricter belief when satisfying a
 * Condition */
struct StricterBond
{
	typedef std::function<bool(const Bond::Values &)> Condition;

	StricterBond(BondConnector &bond, const Condition &cond, 
             Bond::Values impose) :
	_condition(cond), _bond(bond), _impose(impose)
	{
		auto self_check = [this](void *prev) { return check(prev); };

		/* add this object's constraint check function to the connector */
		_bond.add_constraint_check(self_check);

		auto forget_me = [this](void *blame) { return forget(blame); };

		_bond.add_forget(forget_me);

		if (!check(this))
		{
			_bond.pop_last_check(this);

			throw std::runtime_error("New stricter condition immediately "\
			                         "failed validation check");

		}
	}
	
	void forget(void *blame)
	{
		_bond.forget(blame);
	}
	
	bool check(void *previous)
	{
		if (_condition(_bond.value()))
		{
			_bond.assign_value(_impose, this, previous);
		}
		
		return !is_contradictory(_bond.value());
	}
	
	Condition _condition;
	BondConnector &_bond;
	Bond::Values _impose;
};

};

#endif

