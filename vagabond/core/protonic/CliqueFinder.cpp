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

#include "CliqueFinder.h"
#include "Probe.h"
#include "Network.h"
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/ResidueRange.h>

OpSet<Probe *> CliqueFinder::
completeOnCondition(const OpSet<Probe *> &start,
                    std::function<void(Probe *probe)> initial_assessment,
                    std::function<bool(Probe *probe, Probe *prev)> check_probe,
                    int max_jumps)
{
	OpSet<Probe *> done = start;

	for (Probe *probe : done)
	{
		if (probe->atom())
		{
			initial_assessment(probe);
		}
	}

	// go through selected probes and add neighbours if they are in the set.
	// continue until neighbours are exhausted
	int n = 0;
	OpSet<Probe *> fresh = done;
	do
	{
		OpSet<Probe *> tmp = fresh;
		fresh = {};
		for (Probe *probe : tmp)
		{
			for (Probe *other : probe->others())
			{
				if (done.count(other) > 0)
				{
					continue;
				}
				
				if (!check_probe(other, probe))
				{
					continue;
				}

				done.insert(other);
				fresh.insert(other);
			}
		}
		n++;
	}
	while (fresh.size() > 0 && n < max_jumps);
	
	return done;
}

OpSet<Probe *> remove_absents(const OpSet<Probe *> &done)
{
	return done.filter([](Probe *const &p)
	{
		return !p->is_definitely_not_present();
	});
}

OpSet<Probe *> 
CliqueFinder::expandSelectionToNeighbours(const OpSet<Probe *> &done,
                                          int max_jumps, bool with_covalent)
{
	auto initial_assessment = [](Probe *){};

	auto check_probe = [with_covalent](Probe *other, Probe *prev) -> bool
	{
		if (!with_covalent && other->is_covalent())
		{
			return false;
		}

		if (other->is_definitely_not_present())
		{
			return false;
		}

		return true;
	};
	
	OpSet<Probe *> ps = 
	CliqueFinder::completeOnCondition(done, initial_assessment,
	                                  check_probe, max_jumps);
	
	return ps;
}

std::vector<OpSet<Probe *>>
CliqueFinder::probeGroupsForResidues(Network &network,
                                     const std::vector<ResidueRangeToken> &tokens)
{
	std::string firstChain;
	if (network.atoms()->size() > 0)
	{
		firstChain = (*network.atoms())[0]->chain();
	}

	std::vector<OpSet<Probe *>> groups;

	for (const ResidueRangeToken &token : tokens)
	{
		std::string chain = token.chain.empty() ? firstChain : token.chain;

		for (int num = token.begin; num <= token.end; num++)
		{
			ResidueId id(num);
			OpSet<Probe *> group;

			for (AtomProbe *const &probe : network.atomProbes())
			{
				if (probe->atom() && probe->atom()->chain() == chain &&
				    probe->atom()->residueId() == id)
				{
					group.insert(probe);
				}
			}

			groups.push_back(group);
		}
	}

	return groups;
}

OpSet<Probe *>
CliqueFinder::connectGroups(const std::vector<OpSet<Probe *>> &groups,
                            int max_jumps, bool with_covalent)
{
	std::vector<OpSet<Probe *>> reached;
	reached.reserve(groups.size());

	for (const OpSet<Probe *> &group : groups)
	{
		reached.push_back(expandSelectionToNeighbours(group, max_jumps,
		                                              with_covalent));
	}

	OpSet<Probe *> result;
	for (const OpSet<Probe *> &group : groups)
	{
		result += group;
	}

	OpSet<Probe *> all;
	for (const OpSet<Probe *> &r : reached)
	{
		all += r;
	}

	for (Probe *const &probe : all)
	{
		int hits = 0;
		for (const OpSet<Probe *> &r : reached)
		{
			if (r.count(probe))
			{
				hits++;
			}
		}

		if (hits >= 2)
		{
			result.insert(probe);
		}
	}

	return result;
}

OpSet<Probe *> CliqueFinder::findOneClique(const OpSet<Probe *> &all)
{
	for (Probe *const &trial : all)
	{
		OpSet<Probe *> clique = expandSelectionToNeighbours({trial});
		return clique;
	}

	return {};
}

void CliqueFinder::completeAndChop(const OpSet<Probe *> &done,
                                   const HandleClique &handle_clique)
{
	OpSet<Probe *> all = remove_absents(expandSelectionToNeighbours(done));
	OpSet<Probe *> remaining = all;

	while (remaining.size())
	{
		OpSet<Probe *> total = findOneClique(remaining);
		OpSet<Probe *> clique = total.common_to_both(all);

		_cliques.push_back(clique);
		
		if (clique.size() == 0)
		{
			break;
		}

		remaining -= clique;

		if (handle_clique)
		{
			handle_clique(clique, "");
		}
	}
	
	handle_clique(all, "all");
	
	OpSet<Probe *> protein = all;
	protein.filter([](Probe *const &probe)
	{
		return (probe->is_atom() && probe->atom()->bondLengthCount() > 0
		        && probe->atom()->elementSymbol() != "H"
		        && probe->atom()->elementSymbol() != "C");
	});
	
	handle_clique(protein, "protein");
}
