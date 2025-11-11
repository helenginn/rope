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

#ifndef __vagabond__LinkBondPresence__
#define __vagabond__LinkBondPresence__

#include "hnet.h"

namespace hnet
{
/* linking the hydrogen bond breakage status to presence status */
class LinkBondPresence
{
public:
	LinkBondPresence(ExistenceConnector &exist, BondConnector &bond)
	: _exist(exist), _bond(bond)
	{
		prep_constraints_and_forgets(this, {&exist, &bond});
	}

	std::string desc()
	{
		std::ostringstream ss;
		ss << "direct link between existence and bond status for " << _bond;
		return ss.str();
	}

	void forget(OpSet<void *> &blame)
	{
		_exist.forget(blame);
		_bond.forget(blame);
	}

	bool check(void *previous)
	{
		auto assign = make_assign_and_say(this, previous);

		if (_exist.value() == Existence::Present)
		{
			assign(_bond, Bond::NotBroken);
		}

		if (_exist.value() == Existence::Absent)
		{
			assign(_bond, Bond::Broken);
		}

		if (!(_bond.value() & Bond::Broken))
		{
			assign(_exist, Existence::Present);
		}

		/*
		if ((_bond.value() & Bond::Broken))
		{
			assign(_exist, Existence::Absent);
		}
		*/

		return assign.okay();
	}
private:

	ExistenceConnector &_exist;
	BondConnector &_bond;
};
};

#endif
