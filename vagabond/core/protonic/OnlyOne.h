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

#ifndef __vagabond__OnlyOne__
#define __vagabond__OnlyOne__

#include "hnet.h"
#include "Connector.h"
#include "ConstraintBase.h"

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct OnlyOne : public ConstraintBase
{
	OnlyOne(std::vector<ExistenceConnector *> exclusives, 
	        bool must_be_one = true)
	: _exclusives(exclusives), _must_be_one(must_be_one)
	{
		std::vector<ConnectBase *> list;
		for (ExistenceConnector *conn : exclusives)
		{
			list.push_back(conn);
		}
		prep_constraints_and_forgets(this, list);
	}
	
	std::string desc()
	{
		std::ostringstream ss;
		if (_must_be_one)
		{
			ss << "exactly one of [";
		}
		else
		{
			ss << "no more than one of [";
		}
		
		for (ExistenceConnector *conn : _exclusives)
		{
			ss << *conn << ", ";
		}
		std::string str = ss.str();
		str.pop_back();
		str.pop_back();
		str += "] should exist";

		return str;

	}

	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		int number_excluding_absences = _exclusives.size();
		bool declare_absent = false;
		for (ExistenceConnector *&existence : _exclusives)
		{
			if (existence->value() == Existence::Present)
			{
				declare_absent = true;
			}
			if (existence->value() == Existence::Absent)
			{
				number_excluding_absences--;
			}
		}

		for (ExistenceConnector *&existence : _exclusives)
		{
			if (existence->value() != Existence::Present && declare_absent)
			{
				assign(*existence, Existence::Absent);
			}
			if (existence->value() != Existence::Absent && 
			    number_excluding_absences == 1 && _must_be_one)
			{
				assign(*existence, Existence::Present);
			}
		}
		
		return assign.okay();
	}

	std::vector<ExistenceConnector *> _exclusives;
	bool _must_be_one{true};
};
};

#endif
