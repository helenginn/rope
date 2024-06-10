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

#ifndef __vagabond__Selection__
#define __vagabond__Selection__

#include <set>

class Atom;
class Route;
class PairwiseDeviations;

class Selection
{
public:
	Selection(Route *const &route);

	void addDeviations(PairwiseDeviations *dev)
	{
		_deviations.push_back(dev);
	}

	void clearFilters(bool allow);
	void addFilter(const std::set<Atom *> &list, bool allow);
private:
	Route *_route = nullptr;
	std::vector<PairwiseDeviations *> _deviations;

};

#endif
