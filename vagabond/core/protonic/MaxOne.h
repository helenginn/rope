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
#include <vector>

namespace hnet
{
// at most one of exclusives may exist - the OnlyOne(exclusives, false)
// mode used to provide this too (see OnlyOne's own comment for why that
// mode was dropped in favour of this, generalised from a fixed pair to
// any number of exclusives).
struct MaxOne : public ConstraintBase
{
public:
	MaxOne(std::vector<ExistenceConnector *> exclusives)
	: _exclusives(exclusives)
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
		ss << "no more than one of [";

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

		bool declare_absent = false;
		for (ExistenceConnector *&existence : _exclusives)
		{
			if (existence->value() == Existence::Present)
			{
				declare_absent = true;
				break;
			}
		}

		if (declare_absent)
		{
			for (ExistenceConnector *&existence : _exclusives)
			{
				if (existence->value() != Existence::Present)
				{
					assign(*existence, Existence::Absent);
				}
			}
		}

		return assign.okay();
	}
private:
	std::vector<ExistenceConnector *> _exclusives;
};
};

#endif
