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

#ifndef __vagabond__CliqueFinder__
#define __vagabond__CliqueFinder__

class Probe;
#include <vagabond/utils/OpSet.h>
#include <functional>
#include <climits>

class CliqueFinder
{
public:
	static OpSet<Probe *> 
	completeOnCondition(const OpSet<Probe *> &start,
	                    std::function<void(Probe *probe)> initial_assessment,
	                    std::function<bool(Probe *probe, Probe *prev)> 
	                    check_probe, int max_jumps = INT_MAX);

	static 
	OpSet<Probe *> expandSelectionToNeighbours(const OpSet<Probe *> &done,
	                                           const OpSet<Probe *> &all = {},
	                                           int max_jumps = INT_MAX);
	
	typedef std::function<void(const OpSet<Probe *> &,
	                           const std::string &)> HandleClique;
	
	void completeAndChop(const OpSet<Probe *> &done,
	                     const HandleClique &handle_clique = {});
private:
	OpSet<Probe *> findOneClique(const OpSet<Probe *> &all);

	std::vector<OpSet<Probe *>> _cliques;

};

#endif
