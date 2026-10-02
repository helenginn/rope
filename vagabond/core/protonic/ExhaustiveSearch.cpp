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

#include "ExhaustiveSearch.h"
#include "CertainStates.h"
#include "CountProbe.h"
#include "Energy.h"
#include "Network.h"
#include <random>
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>

ExhaustiveSearch::ExhaustiveSearch(const OpSet<Probe *> &interesting,
                                   const OpSet<Probe *> &wider)
: _all(interesting), _wider(wider)
{
	_wider += interesting;

	// only probes still uncertain at the outset are worth re-checking as
	// the search proceeds - anything already certain here is a constant
	// for every result, so recording it per-result would just add
	// meaningless columns to the correlation data.
	_wider.filter([](Probe *const &probe)
	{
		return !probe->is_certain();
	});

	for (Probe *const &probe : _wider)
	{
		if (!probe->energyWrappers().empty())
		{
			_energyProbes.push_back(probe);
		}
	}
}

namespace
{

enum class DecreeOrder
{
	Connected,
	Random,
};

// ROPE_DECREE_ORDER=random restores the old random existence-decree order,
// for comparing search times and arrangement counts (which must not differ -
// the search is exhaustive); anything else (or unset) is connectivity order.
DecreeOrder decree_order()
{
	static DecreeOrder order = []()
	{
		const char *env = std::getenv("ROPE_DECREE_ORDER");
		bool random = env && std::string(env) == "random";
		return random ? DecreeOrder::Random : DecreeOrder::Connected;
	}();

	return order;
}

// maximum-cardinality ordering of the probe graph (edges: Probe::others(),
// restricted to `all`): repeatedly takes the unplaced probe with the most
// already-placed neighbours, so each probe comes right after the ones it is
// connected to - the standard way to order variables for a backtracking
// search over a constraint graph. Ties go to the probe with more neighbours
// overall, then to desc() order, so the result is the same every run
// (pointer order is not). Hydrogens - which link the two half-bonds of an
// H-bond and are not decreed themselves - take part, so atoms either side
// of one end up adjacent. Returns each probe's position.
std::map<Probe *, int> connected_rank(const OpSet<Probe *> &all)
{
	std::vector<Probe *> nodes(all.begin(), all.end());
	size_t n = nodes.size();

	std::map<Probe *, size_t> index;
	std::vector<std::string> descs(n);
	for (size_t i = 0; i < n; i++)
	{
		index[nodes[i]] = i;
		descs[i] = nodes[i]->desc();
	}

	// symmetric adjacency, without duplicates or self-edges
	std::vector<std::vector<size_t>> neighbours(n);
	for (size_t i = 0; i < n; i++)
	{
		for (Probe *other : nodes[i]->others())
		{
			auto it = index.find(other);
			if (it == index.end() || it->second == i)
			{
				continue;
			}

			neighbours[i].push_back(it->second);
			neighbours[it->second].push_back(i);
		}
	}

	for (std::vector<size_t> &list : neighbours)
	{
		std::sort(list.begin(), list.end());
		list.erase(std::unique(list.begin(), list.end()), list.end());
	}

	std::vector<int> placedNeighbours(n, 0);
	std::vector<bool> placed(n, false);
	std::map<Probe *, int> rank;

	for (size_t step = 0; step < n; step++)
	{
		size_t best = n;
		for (size_t i = 0; i < n; i++)
		{
			if (placed[i])
			{
				continue;
			}

			if (best == n)
			{
				best = i;
				continue;
			}

			bool better = false;
			if (placedNeighbours[i] != placedNeighbours[best])
			{
				better = placedNeighbours[i] > placedNeighbours[best];
			}
			else if (neighbours[i].size() != neighbours[best].size())
			{
				better = neighbours[i].size() > neighbours[best].size();
			}
			else
			{
				better = descs[i] < descs[best];
			}

			if (better)
			{
				best = i;
			}
		}

		placed[best] = true;
		rank[nodes[best]] = (int)step;
		for (size_t other : neighbours[best])
		{
			placedNeighbours[other]++;
		}
	}

	return rank;
}

}

void ExhaustiveSearch::setup()
{
	hnet::ConnectBase::_silent = true;

	auto make_bond_decree = [this](BondProbe *probe) -> IteratedProbe *
	{
		if (probe->_obj.is_certain()) { return nullptr; }
		if (probe->is_covalent()) { return nullptr; };
		std::vector<hnet::Bond::Values> options;
		options = probe->_obj.values();
		IteratedProbe *ip = 
		new IterateDecree<hnet::BondConnector, hnet::Bond::Values>
		(probe, probe->_obj, probe->_exist, options, "bond");
		return ip;
	};

	auto make_exist_atom_decree = [this]<class Existing>(Existing *probe)
	-> IteratedProbe *
	{
		if (probe->_exist.is_certain()) { return nullptr; }
		std::vector<hnet::Existence::Values> options = probe->_exist.values();
		IteratedProbe *ip = 
		new IterateDecree<hnet::ExistenceConnector, hnet::Existence::Values>
		(probe, probe->_exist, probe->_exist, options, "exist");
		return ip;
	};

	auto make_exist_bond_decree = [this]<class Existing>(Existing *probe)
	-> IteratedProbe *
	{
		if (probe->_exist.is_certain()) { return nullptr; }
		if (probe->is_covalent()) { return nullptr; };
		std::vector<hnet::Existence::Values> options = probe->_exist.values();
		IteratedProbe *ip =
		new IterateDecree<hnet::ExistenceConnector, hnet::Existence::Values>
		(probe, probe->_exist, probe->_exist, options, "exist");
		return ip;
	};

	// a charge probe's own existence is already covered by its atom's
	// AtomProbe (both reached independently via _all, for the shared
	// case both atoms' - see setupHistidine()/setupCarboxylOxygen() in
	// Network.cpp), so only its value (_obj, which state of charge) ever
	// needs its own decree here, same as make_bond_decree() below for a
	// bond's kind.
	auto make_charge_decree = [this](CountProbe *probe) -> IteratedProbe *
	{
		if (probe->_obj.is_certain()) { return nullptr; }
		std::vector<hnet::Count::Values> options = probe->_obj.values();
		IteratedProbe *ip =
		new IterateDecree<hnet::CountConnector, hnet::Count::Values>
		(probe, probe->_obj, probe->_exist, options, "charge");
		return ip;
	};

	// each decree remembers which probe it belongs to, so the three groups
	// below can be put in connectivity order (see connected_rank()).
	typedef std::pair<Probe *, IteratedProbe *> Decree;
	std::vector<Decree> existDecrees, kindDecrees, chargeDecrees;

	for (Probe *const &probe : _all)
	{
		if (probe->is_bond())
		{
			BondProbe *bp = static_cast<BondProbe *>(probe);
			{
				IteratedProbe *ip = make_exist_bond_decree(bp);
				if (ip) { existDecrees.push_back({probe, ip}); }
			}
		}
		else if (probe->is_atom())
		{
			AtomProbe *ap = static_cast<AtomProbe *>(probe);
			{
				IteratedProbe *ip = make_exist_atom_decree(ap);
				if (ip) { existDecrees.push_back({probe, ip}); }
			}
		}
	}

	for (Probe *const &probe : _all)
	{
		if (probe->is_bond())
		{
			BondProbe *bp = static_cast<BondProbe *>(probe);
			IteratedProbe *ip = make_bond_decree(bp);
			if (ip) { kindDecrees.push_back({probe, ip}); }
		}
	}

	// charge decrees last, after bond kind: a shared/straightforward
	// charge (Coordinated::finishOptions()'s _options ties geometry,
	// charge and outershell_neutral_e together as one combinatorial
	// choice) is only genuinely underdetermined once the atom's own
	// existence and bond pattern have settled - e.g. a carboxylate's
	// {Three, Zero, Six} vs {Three, mOne, Six} states share the same
	// geometry/neutral-electron count and only differ in charge, so
	// nothing else here would ever pin it down without its own decree.
	for (Probe *const &probe : _all)
	{
		if (probe->is_charge())
		{
			CountProbe *cp = static_cast<CountProbe *>(probe);
			IteratedProbe *ip = make_charge_decree(cp);
			if (ip) { chargeDecrees.push_back({probe, ip}); }
		}
	}

	// the order decrees are assigned in is the order of the backtracking
	// search: a decree whose neighbours are already settled has its
	// constraints checked straight away, so a contradiction is found as
	// soon as it exists rather than after enumerating every combination of
	// unrelated decrees assigned in between. Existence decrees used to be
	// shuffled at random, which scattered neighbours far apart.
	// "random" restores that (ROPE_DECREE_ORDER=random, for comparing).
	// The groups keep their relative order (existence, then bond kind, then
	// charge - see above); only the order within each group changes.
	if (decree_order() == DecreeOrder::Random)
	{
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(existDecrees.begin(), existDecrees.end(), g);
	}
	else
	{
		std::map<Probe *, int> rank = connected_rank(_all);
		auto by_rank = [&rank](const Decree &a, const Decree &b)
		{
			return rank.at(a.first) < rank.at(b.first);
		};

		std::stable_sort(existDecrees.begin(), existDecrees.end(), by_rank);
		std::stable_sort(kindDecrees.begin(), kindDecrees.end(), by_rank);
		std::stable_sort(chargeDecrees.begin(), chargeDecrees.end(), by_rank);
	}

	for (const std::vector<Decree> *group :
	     {&existDecrees, &kindDecrees, &chargeDecrees})
	{
		for (const Decree &decree : *group)
		{
			_iterations.push_back(decree.second);
		}
	}

	hnet::ConnectBase::_silent = true;
}
	
void ExhaustiveSearch::cleanup()
{
	_states = std::make_shared<CertainStates>(_results);
	_results.clear();

	for (IteratedProbe *it : _iterations)
	{
		it->reset(_cv, _m);
		delete it;
	}
	hnet::ConnectBase::_silent = false;
}

void ExhaustiveSearch::search()
{
	auto begin = std::chrono::steady_clock::now();
	setup();

	bool interrupted = false;
	while (true)
	{
		if (_cancel && _cancel->load())
		{
			interrupted = true;
			break;
		}

		if (_skip && _skip->load())
		{
			interrupted = true;
			break;
		}

		if (!next())
		{
			break;
		}
	}

	// for comparing decree orderings (ROPE_DECREE_ORDER) - the search is
	// exhaustive, so the arrangement count must be the same either way
	// unless it was cancelled or skipped part-way.
	long ms = (long)std::chrono::duration_cast<std::chrono::milliseconds>
	(std::chrono::steady_clock::now() - begin).count();
	std::cout << "ExhaustiveSearch: " << _iterations.size() << " decrees, "
	          << _results.size() << " arrangements, " << ms << " ms ("
	          << (decree_order() == DecreeOrder::Random ? "random" :
	              "connected") << " order"
	          << (interrupted ? ", interrupted" : "") << ")" << std::endl;

	cleanup();
}

GetScore ExhaustiveSearch::score_wider_clique()
{
	// a fresh id per call (not the fixed, never-updated _gv) - this is
	// what lets a shared, memoised EnergyWrapper (e.g. a repulsion term
	// bundled across alt-conf siblings) tell "still the same scoring
	// round, a different sibling already counted this" apart from "a
	// new round, count it again".
	GuiltVersion gv = Guilt::issueNext();

	// Every wider probe's energy terms are evaluated now, for this
	// configuration, and folded into an EnergyTally: the baked terms (all but
	// protonation) become one running sum per energy source, rather than one
	// stored function object per term. A configuration used to keep every
	// term of the whole wider clique - thousands of heap-allocated
	// std::functions - and replay them all on every score(i) call, which
	// dominated both the memory of a CertainStates and the time to read its
	// scores back. The sums are still gated by the live source flags and
	// amplifiers, so the energy tickboxes and sliders act on them as before.
	// Only probes that have any wrappers are visited, in _wider's order.
	hnet::EnergyTally tally;
	for (Probe *const &probe : _energyProbes)
	{
		for (const hnet::EnergyWrapper &wrapper : probe->energyWrappers())
		{
			tally.add(wrapper(gv));
		}
	}

	return tally.score();
}

bool ExhaustiveSearch::next()
{
	if (_iterations.size() <= 1)
	{
		return false;
	}

	// "certain" means every decree in this config has settled on exactly
	// one option (a single set bit) - std::popcount (a hardware popcount
	// instruction on any remotely modern CPU) replaces what was a manual
	// 32-iteration bit-by-bit count, on the single hottest check in this
	// whole search (run once per call to next(), which runs once per
	// enumerated configuration).
	auto check_certainty = [](const Config &c)
	{
		for (const unsigned int &i : c)
		{
			if (std::popcount(i) != 1)
			{
				return false;
			}
		}
		return true;
	};
	
	
	// read once per decree (it used to be read twice, the first result
	// discarded) into a buffer that lives across calls, rather than a fresh
	// vector every time; add_result() copies it only when it is a new
	// configuration worth keeping.
	auto as_configuration = [this]() -> const Config &
	{
		_scratchConfig.resize(_iterations.size());
		size_t i = 0;
		for (IteratedProbe *ip : _iterations)
		{
			_scratchConfig[i] = ip->value_as_int();
			i++;
		}
		return _scratchConfig;
	};

	auto add_result = [this, &check_certainty]
	(const Config &c)
	{
		bool cert = check_certainty(c);
		if (cert)
		{
			_certain++;
		}

		// one lookup, not a count() then an insert: true only if c is new
		if (cert && _configs.insert(c).second)
		{

			// built here, not in a later deferred pass: by the time any
			// pass over all of _configs would run, the search has moved
			// on and the live connector state no longer corresponds to
			// this specific historical config - is_certain()/
			// certainValueAsInt() have to be read while c is still live.
			GetScore score = score_wider_clique();
			ProbeResult result({}, score);

			// _wider only holds probes that were still uncertain at the
			// outset (see the constructor) - anything now certain became
			// so as a side effect of this particular decree combination,
			// covering every decreed probe too (they are a subset of
			// _wider), so nothing else needs recording separately.
			for (Probe *const &probe : _wider)
			{
				if (!probe->is_certain() || probe->is_covalent())
				{
					continue;
				}

				// _wider deliberately still holds placeholder hydrogens/
				// bonds (SearchAll.cpp passes exclude_placeholders=false
				// when building it) so score_wider_clique() above keeps
				// counting them towards the energy sum - but recording
				// them here too would feed them straight into this
				// result's own CertainStates/correlation-matrix headers,
				// which should stay placeholder-free the same way
				// Subdivide::prune() already keeps them out of the
				// decreed/searched set itself.
				if (probe->is_placeholder())
				{
					continue;
				}

				// a bridging hydrogen's own "protonation state" connector
				// (HydrogenProbe::_obj) is neither is_bond() nor is_atom()
				// (both false, unoverridden base Probe defaults) - unlike
				// every other Probe subtype reaching this point, so it
				// needs its own explicit admission here rather than being
				// picked up by either check below. Recorded as
				// ExistenceType (h is an ExistenceConnector, same as
				// AtomProbe's own _exist) - see HBondDiagram::
				// computeStateAverages() for why this is worth recording
				// explicitly rather than only ever inferring it from its
				// two bonds' own BondType values.
				bool isHydrogenProtonation =
				(dynamic_cast<HydrogenProbe *>(probe) != nullptr);

				if (!probe->is_bond() && !probe->is_atom() &&
				    !probe->is_charge() && !isHydrogenProtonation)
				{
					continue;
				}

				if (probe->atom() && probe->atom()->elementSymbol() == "H")
				{
					continue;
				}

				if (probe->is_bond())
				{
					result.addResult({probe, hnet::Types::BondType,
					                  probe->certainValueAsInt()});
				}
				else if (probe->is_charge())
				{
					result.addResult({probe, hnet::Types::ChargeType,
					                  probe->certainValueAsInt()});
				}
				else
				{
					result.addResult({probe, hnet::Types::ExistenceType,
					                  probe->certainValueAsInt()});
				}
			}

			_total += result.results().size();
			_results.push_back(result);
		}
	};
	
	auto iterate_until_okay_and_done = [this]
	(IteratedProbe &iterate)
	{
		bool okay = false;
		do 
		{
			okay = iterate(_cv, _m);
		}
		while (!okay && !iterate.done());

		return iterate.done();
	};
	
	auto it = _iterations.begin();
	for (int i = 0; i < _counter; i++)
	{
		it++;
	}

	auto count_down = [&it, this]()
	{
		while (_counter >= _iterations.size())
		{
			it--;
			_counter--;
		}
		
		while ((**it).done())
		{
			(**it).reset(_cv, _m);
			it--;
			_counter--;
			
			if (_counter == 0)
			{
				break;
			}
		}
	};
	auto count_up = [&it, &count_down, this]()
	{
		it++;
		_counter++;
		if (_counter >= _iterations.size())
		{
			count_down();
		}
	};
	
	IteratedProbe &iterate = **it;
	bool done = iterate_until_okay_and_done(iterate);
	const Config &result = as_configuration();
	add_result(result);
	
	if (iterate.is_certain())
	{
		count_up();
	}
	else if (_counter > 0)
	{
		count_down();
	}
	
	if (_counter < 0 || (_counter == 0 && (**it).done()))
	{
		return false;
	}

	return true;
}

