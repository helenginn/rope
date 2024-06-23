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

#include "Atom.h"
#include "Route.h"
#include "Parameter.h"
#include "Selection.h"
#include "../BondSequenceHandler.h"
#include "../BondSequence.h"
#include "function_typedefs.h"
#include "PairwiseDeviations.h"

Selection::Selection(Route *const &route)
{
	_route = route;
}

auto make_filter_from_list(const std::set<int> &affected, bool allow)
{
	return [allow, affected](int p) -> int
	{
		if (affected.count(p))
		{
			return allow;
		}

		return -1;
	};
}

PairFilter filterForAtoms(PairwiseDeviations *dev, 
                          const std::set<Atom *> &list, bool allow) 
{
	std::set<int> affected;
	for (Atom *const &atom : list)
	{
		int idx = dev->index(atom);
		if (idx >= 0)
		{
			affected.insert(idx);
		}
	}
	
	return make_filter_from_list(affected, allow);
}

PairFilter filterForParameters(Route *route, const std::set<Atom *> &list, 
                               bool allow)
{
	std::set<int> affected;

	for (int i = 0; i < route->motionCount(); i++)
	{
		Parameter *param = route->parameter(i);

		for (int j = 0; j < param->atomCount(); j++)
		{
			if (list.count(param->atom(j)) > 0)
			{
				affected.insert(i);
				break;
			}
		}
	}
	std::cout << "Affected params: " << affected.size() << std::endl;

	return make_filter_from_list(affected, allow);
}

PairFilter addFilters(const PairFilter &old_filter,
                      const PairFilter &new_filter)
{
	PairFilter combined = [new_filter, old_filter](const int &p)
	{
		int result = new_filter(p);
		if (result >= 0)
		{
			return result;
		}
		else if (old_filter)
		{
			return old_filter(p);
		}
		else
		{
			return 1;
		}
	};
	
	return combined;
}


void Selection::addFilter(const std::set<Atom *> &list, bool allow)
{
	PairFilter addition = filterForParameters(_route, list, allow);
	_route->_motionFilter = addFilters(_route->_motionFilter, addition);

	for (PairwiseDeviations *const &chosen : _deviations)
	{
		if (chosen)
		{
			PairFilter oldFilter = chosen->filter();
			PairFilter atomFilter = filterForAtoms(chosen, list, allow);
			PairFilter added = addFilters(oldFilter, atomFilter);
			chosen->setFilter(added);
		}
	}
}

void Selection::clearFilters(bool allow)
{
	PairFilter filter = [allow](const int &) -> int { return allow; };
	_route->_motionFilter = filter;

	for (PairwiseDeviations *const &chosen : _deviations)
	{
		if (chosen)
		{
			chosen->setFilter(filter);
		}
	}
}

typedef std::pair<int, int> IntInt;

std::vector<IntInt>
Selection::activeParameters(BondSequenceHandler *const &handler,
                            const ValidateIndex &validate)
{
	const std::vector<AtomBlock> &blocks = handler->sequence()->blocks();
	TorsionBasis *basis = handler->torsionBasis();

	std::vector<IntInt> idxs;

	for (size_t bidx = 0; bidx < blocks.size(); bidx++)
	{
		if (blocks[bidx].torsion_idx < 0) { continue; }
		int tidx = blocks[bidx].torsion_idx;

		Parameter *p = basis->parameter(tidx);
		int pidx = _route->indexOfParameter(p);
		if (validate && !validate(pidx))
		{
			continue;
		}
		if (p->isConstrained())
		{
			continue;
		}
		if (_route->_motionFilter && !_route->_motionFilter(pidx))
		{
			continue;
		}

		idxs.push_back({bidx, pidx}); // block index, param (also motion) index
	}

	return idxs;
}
