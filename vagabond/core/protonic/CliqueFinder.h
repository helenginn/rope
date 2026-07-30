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
class Network;
struct ResidueRangeToken;
#include <vagabond/utils/OpSet.h>
#include <functional>
#include <climits>
#include <vector>

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
	                                           int max_jumps = INT_MAX,
	                                           bool with_covalent = false);

	/** one group of AtomProbes per named residue (a chain == "" token is
	 * resolved to the model's first chain). Used by connectGroups() to
	 * tell "reaches a different named residue" apart from "just another
	 * atom of the same residue". */
	static std::vector<OpSet<Probe *>>
	probeGroupsForResidues(Network &network,
	                       const std::vector<ResidueRangeToken> &tokens);

	/** grows each group independently by up to max_jumps hops (reusing
	 * expandSelectionToNeighbours per group) and keeps a probe only if it
	 * belongs to one of the named residues itself, or is reachable within
	 * max_jumps hops from at least two *different* groups - i.e. it
	 * actually bridges two of the named residues, rather than merely
	 * being near any single one of them. */
	static OpSet<Probe *>
	connectGroups(const std::vector<OpSet<Probe *>> &groups,
	             int max_jumps = INT_MAX, bool with_covalent = false);

	typedef std::function<void(const OpSet<Probe *> &,
	                           const std::string &)> HandleClique;

	void completeAndChop(const OpSet<Probe *> &done,
	                     const HandleClique &handle_clique = {});
private:
	OpSet<Probe *> findOneClique(const OpSet<Probe *> &all);

	std::vector<OpSet<Probe *>> _cliques;

};

#endif
