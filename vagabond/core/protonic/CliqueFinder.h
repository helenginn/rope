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

	/** exclude_placeholders (on by default) drops placeholder hydrogens/
	 * bonds (Probe::is_placeholder()) from the walk - they're speculative,
	 * not-yet-resolved coordination slots that shouldn't be offered as
	 * part of a chosen region of interest for analysis, nor searched by
	 * ExhaustiveSearch. SearchAll's own energy-widening expansion passes
	 * false here so they still contribute to the energy calculation. */
	static
	OpSet<Probe *> expandSelectionToNeighbours(const OpSet<Probe *> &done,
	                                           int max_jumps = INT_MAX,
	                                           bool with_covalent = false,
	                                           bool exclude_placeholders = true);

	/** walks covalent bonds only, filling in the rest of every residue
	 * already represented in `start` (never reaching into a *new*
	 * residue that wasn't already there) - shared by
	 * ProtonNetworkView::completeResidues() (the "complete residues"/
	 * "complete to C-alpha" menu options) and anything else that wants
	 * the same "fill in this residue's full atom set" behaviour, e.g. to
	 * give SearchAll's own covalent-inclusive _wider expansion something
	 * to reach through beyond an H-bond-only selection. stop_at_alpha
	 * stops each residue's completion at its reporter atom (Cα) rather
	 * than continuing on into the next residue's backbone. */
	static OpSet<Probe *> completeToResidues(const OpSet<Probe *> &start,
	                                         bool stop_at_alpha = false);

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
};

#endif
