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

#ifndef __vagabond__OrCount__
#define __vagabond__OrCount__

#include "ConstraintBase.h"

namespace hnet
{
/* takes a bunch of counts and sets the sum equal to any of the bunch's
 * options */
struct OrCount : public ConstraintBase
{
public:
	OrCount(std::vector<CountConnector *> connectors, CountConnector &sum)
	: _sum(sum), _bunch(connectors)
	{
		std::vector<ConnectBase *> base;
		for (CountConnector *cc : connectors)
		{
			base.push_back(cc);
		}
		base.push_back(&sum);
		prep_constraints_and_forgets(this, base);
	}

	std::string desc()
	{
		return "OR operator for counts for " + _sum.desc();
	}

	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);
		
		// first we check the covalent bond and what effect that has on count
		
		Count::Values tmp_all = Count::Contradiction;
		
		for (CountConnector *cc : _bunch)
		{
			tmp_all = (Count::Values)(tmp_all | cc->value());
		}
		
		assign(_sum, tmp_all, "OR of all counts");
		Count::Values current = _sum.value();
		
		for (CountConnector *cc : _bunch)
		{
			assign(*cc, current);
		}

		return assign.okay();
	}
private:
	CountConnector &_sum;
	std::vector<CountConnector *> _bunch;

};
};

#endif
