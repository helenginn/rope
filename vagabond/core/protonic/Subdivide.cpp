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

#include "Subdivide.h"
#include "Probe.h"
#include "Clique.h"
#include <algorithm>
#include <random>
#include <queue>

Subdivide::Subdivide(Clique *clique, int max) : _clique(clique)
{
	_max = max;
	int actual_max = _clique->probes().size();
	if (_max > actual_max)
	{
		_max = actual_max;
	}
}

// true for a symmetry-mate atom (Atom::symmetryCopyOf() non-null) or a
// bond touching one - mirrors Clique.cpp's own is_symmetry_mate(), which
// can't be reused directly since it lives in that file's anonymous
// namespace. Crystallographic symmetry mates are folded into the same
// probe graph Network builds (see Network::Network()) so they resolve
// their own H-bonding independently, but a subdivision chunk searched by
// ExhaustiveSearch is meant to represent one physically real, contiguous
// region of the asymmetric unit - pulling in a mate atom or a bond to one
// mixes in a duplicate, independently-resolving copy of that region
// instead of extending it. Unlike placeholders (Probe::is_placeholder(),
// filtered only from the RESULT - see prune()'s own comment), a symmetry
// mate is excluded from the WALK itself: it is never a bridge to some
// other real, distinct region the way a placeholder can be, only ever a
// duplicate of a region already reachable through the real atom instead.
static bool is_symmetry_related(Probe *probe)
{
	if (probe->atom())
	{
		return probe->atom()->symmetryCopyOf() != nullptr;
	}

	if (probe->is_bond())
	{
		BondProbe *bp = static_cast<BondProbe *>(probe);
		Atom *l = bp->left().atom();
		Atom *r = bp->right().atom();
		return (l && l->symmetryCopyOf()) || (r && r->symmetryCopyOf());
	}

	return false;
}

bool Subdivide::finish_ends(OpSet<Probe *> &chunk)
{
	// catches every direct (non-bond-mediated) atom<->atom edge on an
	// atom already in the chunk - not just alt-conf siblings despite the
	// name (Network::establishAtom()), but also steric clashes
	// (Coordinated::clashLogic()) and charge-sharing/tautomer partners
	// (Network::shareProperty()), all of which are registered the same
	// way. All three need joint existence-constraint resolution the same
	// way alt-conf siblings do, so all three belong in the same
	// subdivision chunk together - deliberately NOT narrowed to
	// alt-confs specifically. This is separate from, and much broader
	// than, Probe::bondedNeighbours() (the GUI's 2D-layout weighting),
	// which deliberately excludes every one of these same edges - see
	// its own comment for why. Also pulls in an atom's own CountProbe
	// (its charge, shared or single-atom - see add_charge_display() in
	// Coordinated_Core.cpp and setupHistidine()/setupCarboxylOxygen() in
	// Network.cpp) wherever one was registered into others(): skipped
	// only for an atom whose charge was merged into a shared CountProbe
	// elsewhere (Network::shareCharges() - Histidine, carboxylates), not
	// for a merely ambiguous, unmerged one (e.g. Arginine's).
	//
	// Deliberately atoms-only: hydrogen-bond completion (protonation
	// states and the opposing heavy atom) is finish_hbonds()'s job, kept
	// as a separate pass run to its own fixpoint AFTER this one finishes
	// (see subdivide()'s grow_clique()) rather than interleaved here -
	// interleaving let each pass's newly-added atoms feed the other
	// (a fresh alt-conf pulling in its own H-bonds, whose opposing atom's
	// own alt-confs pull in more H-bonds, ...), creeping the chunk
	// towards the whole network instead of staying bounded.
	for (Probe *const &probe : chunk)
	{
		if (probe->is_atom())
		{
			for (Probe *const &other : probe->others())
			{
				if (other->is_definitely_not_present() ||
				    is_symmetry_related(other))
				{
					continue;
				}

				if (!other->is_atom() && !other->is_charge())
				{
					continue;
				}

				if (chunk.count(other) == 0)
				{
					chunk += other;
					return true;
				}
			}

			// mutual-existence neighbours are deliberately NOT walked
			// here (used to be - see comparison_key()'s own comment for
			// why that moved out): pulling them into the real, searched
			// chunk let the walk chain sideways through others() into
			// clash partners, whose own mutual-existence neighbours could
			// then chain further still - an unbounded, mixed-relation
			// cascade with no natural stopping point, and it bloated the
			// actual subdivisions that get searched. The convergence
			// this was for (so near-duplicate subdivisions can be
			// screened out) is now handled by a same-day, throwaway
			// expansion computed only for that comparison - see
			// subdivide().
		}
	}

	return false;
}

// adds `other` unless it's absent or a symmetry mate - same filter
// finish_ends()/bounded_bfs() apply, factored out here since finish_hbonds()
// below applies it at several different hops of the same leg rather than
// once per call.
static void add_leg_member(OpSet<Probe *> &chunk, Probe *other)
{
	if (!other || other->is_definitely_not_present() ||
	    is_symmetry_related(other))
	{
		return;
	}

	chunk += other;
}

// companion to finish_ends() above, run once (NOT to a fixpoint, and
// strictly AFTER finish_ends() has already finished growing alt-confs/
// clashes to its own fixpoint - see subdivide()'s grow_clique()) rather
// than folded back into it or looped itself. A real H-bond is always
// exactly two BondProbes (non-covalent - see CovalentProbe.h) sharing one
// HydrogenProbe between them, one BondProbe per heavy-atom side (see
// Coordinated_Constraints.cpp's create_two_half_hydrogen_bonds()) - i.e.
// atom -> half -> hydrogen -> half -> atom, four hops end to end. Starting
// from every atom, H-bond half, or hydrogen already in the chunk (a
// snapshot taken up front, so a probe THIS call adds is never itself used
// as a new starting point), this completes only the specific leg(s) that
// probe already sits on, up to that leg's own far atom - never chaining
// past that into whatever OTHER H-bonds the far atom happens to have.
// That's what keeps this a single, bounded pass rather than a walk that
// could otherwise creep across the whole H-bond network: chained fixpoint
// looping is finish_ends()'s job for alt-confs/clashes, not this one's.
void Subdivide::finish_hbonds(OpSet<Probe *> &chunk)
{
	// temporary testing toggle: stop each leg at the hydrogen (protonation
	// state) instead of continuing on to the opposing half/atom, to check
	// whether that's what's driving the combinatorial blow-up before
	// deciding whether the full four-hop leg is too much. Flip back to
	// false to restore the full atom->half->hydrogen->half->atom leg.
	const bool STOP_AT_HYDROGEN = false;

	std::vector<Probe *> snapshot(chunk.begin(), chunk.end());

	for (Probe *const &probe : snapshot)
	{
		std::vector<Probe *> halves;

		if (probe->is_atom())
		{
			for (Probe *const &other : probe->others())
			{
				if (other->is_bond() && !other->is_covalent())
				{
					halves.push_back(other);
				}
			}
		}
		else if (probe->is_bond() && !probe->is_covalent())
		{
			halves.push_back(probe);
		}
		else if (!probe->is_bond()) // hydrogen
		{
			for (Probe *const &other : probe->others())
			{
				if (other->is_bond() && !other->is_covalent())
				{
					halves.push_back(other);
				}
			}
		}
		else
		{
			continue; // covalent bond or charge: not on an H-bond leg
		}

		for (Probe *const &half : halves)
		{
			add_leg_member(chunk, half);

			Probe *hydrogen = nullptr;
			for (Probe *const &other : half->others())
			{
				if (!other->is_atom())
				{
					hydrogen = other;
					break;
				}
			}

			add_leg_member(chunk, hydrogen);
			if (!hydrogen || STOP_AT_HYDROGEN)
			{
				continue;
			}

			for (Probe *const &other_half : hydrogen->others())
			{
				if (other_half == half || !other_half->is_bond() ||
				    other_half->is_covalent())
				{
					continue;
				}

				add_leg_member(chunk, other_half);

				for (Probe *const &far_atom : other_half->others())
				{
					if (far_atom->is_atom())
					{
						add_leg_member(chunk, far_atom);
					}
				}
			}
		}
	}
}

// incremental cost (in log2 units - i.e. a step of N here means the region
// being grown gets roughly 2^N times as combinatorially large) of adding
// `node` to a region that already reached every key of `included` - this
// is what shoot()'s own growth is now budgeted against, in place of a
// plain node count. Categories, per Helen's own guessed weights:
//  - a covalent bond (CovalentProbe) or the atom on its far side: free -
//    a covalently bonded atom was never an independent combinatorial
//    choice to begin with.
//  - a hydrogen (protonation state) or a charge (CountProbe): also free
//    here - its own real cost is charged at the half-hydrogen-bond/atom
//    hop that reaches it, not charged twice.
//  - a half-hydrogen-bond (a non-covalent BondProbe): 2^4 - by far the
//    most expensive single node, since each one is its own existence
//    constraint cluster (h/hExist/le/re - see HydrogenBond).
//  - an atom whose existence is already tied (mutualExistenceNeighbours())
//    to something already reached: free, same reasoning as the covalent
//    case - it isn't a new independent choice either.
//  - any other atom (an independent existence choice - includes ordinary
//    steric-clash partners): 2^1.
static int node_penalty(Probe *node, const std::map<Probe *, int> &included)
{
	if (node->is_covalent())
	{
		return 0;
	}

	if (node->is_bond())
	{
		return 4;
	}

	if (!node->is_atom())
	{
		return 0; // hydrogen or charge
	}

	for (Probe *const &mate : node->mutualExistenceNeighbours())
	{
		if (included.count(mate))
		{
			return 0;
		}
	}

	return 1;
}

// Dijkstra out from `root`, weighting each step by node_penalty() rather
// than a flat 1 hop, and stopping expansion once a node's own cumulative
// cost reaches `budget` (a log2-penalty threshold, not a hop count - see
// node_penalty()'s own comment for the unit). Fills `dist` with every
// reached probe's cheapest cumulative cost from `root` and returns the
// highest cost actually reached (capped at `budget`). Same
// is_definitely_not_present()/is_symmetry_related() filter the old
// unweighted BFS used.
static int bounded_bfs(Probe *root, int budget, std::map<Probe *, int> &dist)
{
	dist[root] = 0;
	using Entry = std::pair<int, Probe *>;
	std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> frontier;
	frontier.push({0, root});
	int farthest = 0;

	while (!frontier.empty())
	{
		auto [d, current] = frontier.top();
		frontier.pop();

		if (d > dist[current])
		{
			continue; // a cheaper path to `current` was already relaxed
		}

		if (d >= budget)
		{
			continue;
		}

		for (Probe *const &other : current->others())
		{
			if (other->is_definitely_not_present() ||
			    is_symmetry_related(other))
			{
				continue;
			}

			int cost = d + node_penalty(other, dist);

			auto it = dist.find(other);
			if (it != dist.end() && it->second <= cost)
			{
				continue;
			}

			dist[other] = cost;
			farthest = std::max(farthest, cost);
			frontier.push({cost, other});
		}
	}

	return farthest;
}

// Picks a probe roughly _max penalty units (see node_penalty()) from the
// start (biased to the farthest layer at which another member of the
// clique being subdivided is reached, so chunks are deep chains rather
// than the old shuffle-and-backtrack random walk, which tended to
// meander), then keeps the union of every probe within _slack of some
// shortest path between the two - not just one arbitrary shortest path -
// since real signalling paths fork and converge rather than being a
// single strand. The path between them, and the kept "lens" around it,
// may still pass through probes outside the clique (e.g. bridging
// waters) - only the choice of endpoint itself is restricted to clique
// members.
//
// _max is user-set "guidance size" for how big a subdivision should be,
// originally a plain node count - now spent as a log2-penalty budget
// instead (node_penalty()'s own comment has the per-node-type weights),
// so the SAME numeric value is reused unconverted, on the guess that an
// average node costs somewhere around 2^1 (i.e. this budget spends
// roughly _max independent-atom-equivalents rather than literally _max
// nodes) - may need retuning by eye once this is in use. finish_ends()/
// finish_hbonds() no longer draw against this budget at all (they run
// independently, uncapped, after shoot() finishes), so the old halving
// that reserved half of _max as headroom for them is gone too - shoot()
// now spends the whole budget on the path/lens itself.
void Subdivide::shoot(OpSet<Probe *> &chunk)
{
	Probe *start = *chunk.begin();

	int budget = _max;

	std::map<Probe *, int> dist_start;
	bounded_bfs(start, budget, dist_start);

	// the chosen endpoint must itself belong to the clique being
	// subdivided - the walk between start and end is still free to pass
	// through probes outside it (bridging waters etc.), but the endpoint
	// picked to aim at should not be one of those bridging probes. So
	// take the farthest distance at which a clique-member probe was
	// actually reached, rather than the farthest distance reached at
	// all (bounded_bfs's own return value).
	int d = 0;
	for (const auto &entry : dist_start)
	{
		if (entry.second > d && _clique->probes().count(entry.first))
		{
			d = entry.second;
		}
	}

	if (d == 0)
	{
		return; // no other clique member reachable within budget
	}

	std::vector<Probe *> farthest_layer;
	for (const auto &entry : dist_start)
	{
		if (entry.second == d && _clique->probes().count(entry.first))
		{
			farthest_layer.push_back(entry.first);
		}
	}

	static thread_local std::mt19937 rng{std::random_device{}()};

	// bias the endpoint towards whichever candidate(s) in the farthest
	// layer have turned up in the fewest searched chunks so far this
	// subdivide() call, rather than picking uniformly at random - a purely
	// random pick keeps re-visiting whichever nodes happen to be easy to
	// reach from `start`, and as `samples` grows that overweights those
	// nodes' regions relative to ones a random pick rarely lands on. Ties
	// (including the common case where nothing has been sampled yet, i.e.
	// every count is 0) are still broken at random among the least-sampled
	// candidates, so the very first walks behave exactly as before.
	int min_count = -1;
	for (Probe *const &candidate : farthest_layer)
	{
		int count = 0;
		auto it = _nodeCounts.find(candidate);
		if (it != _nodeCounts.end())
		{
			count = it->second;
		}

		if (min_count < 0 || count < min_count)
		{
			min_count = count;
		}
	}

	// if every candidate endpoint in the farthest layer has already turned
	// up in _samples-or-more searched chunks this subdivide() call, this
	// walk has nowhere under-sampled left to aim at - drop the attempt
	// rather than force it onto an already-oversampled endpoint anyway.
	if (_samples > 0 && min_count >= _samples)
	{
		chunk.clear();
		return;
	}

	std::vector<Probe *> least_sampled;
	for (Probe *const &candidate : farthest_layer)
	{
		auto it = _nodeCounts.find(candidate);
		int count = (it == _nodeCounts.end()) ? 0 : it->second;

		if (count == min_count)
		{
			least_sampled.push_back(candidate);
		}
	}

	std::uniform_int_distribution<size_t> pick(0, least_sampled.size() - 1);
	Probe *end = least_sampled[pick(rng)];

	std::map<Probe *, int> dist_end;
	bounded_bfs(end, d + _slack, dist_end);

	// _slack widens the accepted region to the whole "lens" between start
	// and end, not just the path itself, and how many probes fall in that
	// lens depends on local branching, not on _max - so nothing here
	// guarantees the result stays within budget on its own. Rank every
	// candidate by how close it is to an actual shortest path (smallest
	// combined distance first) and keep only the closest _max of them, so
	// the true path (combined distance == d) always survives and the cut
	// falls on the least central candidates. Shuffle first (with the same
	// rng used to pick `end`) so which of several equally-close candidates
	// survive a tie at the cutoff varies across samples, then stable-sort
	// by sum only so that shuffled order is what breaks ties - rather than
	// dist_start's pointer-ordered iteration deciding it by accident.
	struct Candidate { Probe *probe; int sum; };
	std::vector<Candidate> candidates;
	for (const auto &entry : dist_start)
	{
		auto it = dist_end.find(entry.first);
		if (it != dist_end.end() && entry.second + it->second <= d + _slack)
		{
			candidates.push_back({entry.first, entry.second + it->second});
		}
	}

	std::shuffle(candidates.begin(), candidates.end(), rng);
	std::stable_sort(candidates.begin(), candidates.end(),
	                 [](const Candidate &a, const Candidate &b)
	                 {
		                return a.sum < b.sum;
		             });

	// accept candidates closest-first (same ranking as before), spending
	// each one's own node_penalty() against `budget` as it's accepted -
	// this replaces the old plain node-count cap, and stops (rather than
	// skipping over an expensive one to keep packing cheaper, farther-out
	// candidates) at the first candidate that would exceed the budget, so
	// the accepted region stays a genuine "grow outward until the budget
	// runs out" lens rather than a scattered cheapest-first selection.
	OpSet<Probe *> result;
	std::map<Probe *, int> included;
	int spent = 0;

	// the walk's own seed atom is always kept regardless of budget - it's
	// where every one of this shoot() call's own hops originated from,
	// not itself a new combinatorial addition to weigh against it.
	result += start;
	included[start] = 0;

	for (const Candidate &candidate : candidates)
	{
		if (candidate.probe == start)
		{
			continue;
		}

		int cost = node_penalty(candidate.probe, included);

		if (spent + cost > budget)
		{
			break;
		}

		included[candidate.probe] = 0;
		result += candidate.probe;
		spent += cost;
	}

	chunk = result;
}

bool has_non_water(const OpSet<Probe *> &chunk)
{
	for (Probe *const &probe : chunk)
	{
		if (probe->is_atom() && probe->atom()->code() != "HOH")
		{
			return true;
		}
	}

	return false;
}

void Subdivide::prune(OpSet<Probe *> &chunk)
{
	std::erase_if(chunk,
	              [](Probe *const &probe)
	              {
		             // placeholders (Probe::is_placeholder()) are
		             // speculative, not-yet-resolved coordination slots
		             // that don't belong in a searched/stored subdivision -
		             // excluded from Clique/ExhaustiveSearch while still
		             // keeping them in the energy calculation via
		             // SearchAll's wider expansion. Deliberately only
		             // filtered out HERE, after growth, rather than in
		             // finish_ends()/bounded_bfs() above: a placeholder
		             // hydrogen can be the only link between two different
		             // atoms' own candidate hydrogens (the H-H clash
		             // MaxOne/register_probe pair in Coordinated_Hydrogens.
		             // cpp's makePlaceholderHydrogen()), so blocking growth
		             // from stepping onto one at all fragments the walkable
		             // graph into far smaller, disconnected chunks instead
		             // of just omitting it from the result.
		             return probe->is_certain() || probe->is_placeholder();
		          });
}

// throwaway view of a (already pruned) chunk used ONLY to decide whether
// two subdivisions are near-duplicates of each other in subdivide() below
// - never stored, never searched. Placeholder hydrogens/bonds (Probe::
// is_placeholder() - prune() already strips these before this ever runs,
// so this erase_if is a no-op backstop) and covalent bonds don't
// meaningfully distinguish one subdivision from another once the real,
// uncertain H-bond network membership already agrees, so both are
// stripped; mutual-existence neighbours (unambiguous matching-letter-
// conformer covalent bonds - see Probe::registerMutualExistence()'s own
// comment) are added so two subdivisions covering the same ambiguous
// region converge onto the same key regardless of which particular bond
// within it each one's own random walk happened to reach (both resolve
// to functionally identical CertainStates once ExhaustiveSearch runs, but
// a plain membership comparison would otherwise never notice). Only ever
// walks mutualExistenceNeighbours() - never others() - so this can't
// chain sideways into clashes/alt-confs and cascade unboundedly the way
// baking this into finish_ends() itself once did.
static OpSet<Probe *> comparison_key(const OpSet<Probe *> &chunk)
{
	OpSet<Probe *> key = chunk;

	bool added = true;
	while (added)
	{
		added = false;
		for (Probe *const &probe : key)
		{
			if (!probe->is_atom())
			{
				continue;
			}

			for (Probe *const &other : probe->mutualExistenceNeighbours())
			{
				if (other->is_definitely_not_present())
				{
					continue;
				}

				if (key.count(other) == 0)
				{
					key += other;
					added = true;
					break;
				}
			}

			if (added)
			{
				break;
			}
		}
	}

	std::erase_if(key,
	              [](Probe *const &probe)
	              {
		             return probe->is_placeholder() || probe->is_covalent();
		          });

	return key;
}

void Subdivide::one()
{
	OpSet<Probe *> expanded = _clique->probes();
	// same as grow_clique()'s own prune() call in subdivide() below -
	// without this, a clique that only ever goes through one() (never
	// algorithmically subdivided) kept its placeholder hydrogens/bonds
	// (Probe::is_placeholder()) as full members of the resulting
	// subnetwork instead of just contributing to energy scoring, leaking
	// them into ExhaustiveSearch's decreed set and the correlation matrix.
	prune(expanded);
	_clique->addSubdivisionRun({Clique(expanded)}, _max, 0, true);
}

void Subdivide::subdivide(int samples)
{
	auto grow_clique = [this](Probe *start)
	{
		OpSet<Probe *> chunk = {start};
		shoot(chunk);
		while (finish_ends(chunk)) {}
		finish_hbonds(chunk);

		prune(chunk);
		return chunk;
	};

	OpSet<Probe *> to_chunk = _clique->probes();

	// reset per-call so a fresh subdivide() doesn't inherit bias from a
	// previous one, e.g. one() or an earlier subdivide() run on this same
	// Subdivide instance.
	_nodeCounts.clear();
	_samples = samples;

	// A/B toggle, temporary - flip to true to compare against
	// comparison_key()'s (+mutual neighbours, -placeholders, -covalent)
	// view for near-duplicate detection instead of the plain original
	// membership. false reproduces the pre-comparison_key() behaviour
	// exactly (sample.key == sample.original), without deleting that code.
	const bool USE_COMPARISON_KEY = true;

	// each sample keeps its real, searched-and-stored membership
	// (original) separate from the throwaway view used only to spot
	// near-duplicates (key) - see comparison_key()'s own comment for why
	// those must not be the same set. weight starts at 1 (this walk's own
	// share of the sampling) and gains 1 every time another walk turns
	// out to be an exact duplicate or strict subset of this one, instead
	// of that walk's own contribution simply being discarded - see
	// Clique::sampleWeight()'s own comment for why: every probe in
	// to_chunk starts its own independent walk, so a region many walks
	// collapse into represents that many independent "votes" for it, not
	// just one - outright discarding every walk but the first one that
	// reaches a given region silently undercounted it relative to a
	// region only one walk ever reaches. This still only searches (and
	// stores) each surviving region once.
	struct Sample
	{
		OpSet<Probe *> original;
		OpSet<Probe *> key;
		int weight = 1;
	};

	std::vector<Sample> samples_found;
	std::map<OpSet<Probe *>, size_t> keyIndex;

	for (Probe *probe : to_chunk)
	{
		for (int i = 0; i < samples; i++)
		{
			OpSet<Probe *> chunk = grow_clique(probe);
			if (chunk.size() == 0 || !has_non_water(chunk))
			{
				continue;
			}

			// record this chunk's membership so future shoot() calls (from
			// later walks in this same subdivide()) bias their endpoint
			// choice away from nodes already well covered - see shoot()'s
			// own comment for why. Counted here rather than inside
			// grow_clique() itself so a chunk discarded just above (empty,
			// or water-only) doesn't count towards sampling coverage.
			for (Probe *const &member : chunk)
			{
				_nodeCounts[member]++;
			}

			OpSet<Probe *> key = USE_COMPARISON_KEY ?
			comparison_key(chunk) : chunk;

			auto it = keyIndex.find(key);
			if (it != keyIndex.end())
			{
				samples_found[it->second].weight++;
				continue;
			}

			keyIndex[key] = samples_found.size();
			samples_found.push_back({chunk, key});
		}
	}

	// direct (not necessarily maximal - see the walk-up below) superset
	// among every OTHER sample, biased towards the largest candidate
	// found purely so the walk-up chain below tends to be short, not for
	// correctness - transitivity of the subset relation (X subset of Y
	// subset of Z implies X subset of Z) is what actually guarantees every
	// non-maximal sample's chain eventually reaches a genuinely maximal
	// one, however many hops that takes.
	auto find_superset = [&samples_found](size_t idx) -> int
	{
		const Sample &sample = samples_found[idx];
		int best = -1;

		for (size_t i = 0; i < samples_found.size(); i++)
		{
			if (i == idx) { continue; }

			const Sample &other = samples_found[i];
			if (other.key.size() < sample.key.size())
			{
				continue;
			}

			if (std::includes(other.key.begin(), other.key.end(),
			                  sample.key.begin(), sample.key.end()))
			{
				if (best < 0 || other.key.size() >
				    samples_found[best].key.size())
				{
					best = (int)i;
				}
			}
		}

		return best;
	};

	std::vector<int> superset(samples_found.size(), -1);
	for (size_t i = 0; i < samples_found.size(); i++)
	{
		superset[i] = find_superset(i);
	}

	// credits every non-maximal sample's weight to whichever maximal
	// sample actually contains it, walking up the chain first if the
	// direct superset found above isn't itself maximal (see
	// find_superset()'s own comment) - a strict-subset relation can never
	// cycle, so this always terminates.
	for (size_t i = 0; i < samples_found.size(); i++)
	{
		if (superset[i] < 0)
		{
			continue;
		}

		int target = superset[i];
		while (superset[target] >= 0)
		{
			target = superset[target];
		}

		samples_found[target].weight += samples_found[i].weight;
	}

	OpSet<Clique> cliques;
	for (size_t i = 0; i < samples_found.size(); i++)
	{
		const Sample &sample = samples_found[i];
		if (superset[i] >= 0 || sample.original.size() < 2)
		{
			continue;
		}

		Clique cl(sample.original);
		cl.setSampleWeight(sample.weight);
		cliques.insert(cl);
	}

	std::cout << "Found " << cliques.size() << std::endl;

	_clique->setSampleCounts(_nodeCounts);
	_clique->addSubdivisionRun(cliques, _max, samples, false);
}

