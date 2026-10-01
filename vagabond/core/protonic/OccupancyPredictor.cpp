// vagabond
// Copyright (C) 2026 Helen Ginn
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

#include "OccupancyPredictor.h"
#include "Clique.h"
#include "ProbeResult.h"
#include "CertainStates.h"
#include "OccupancyPropagation.h"
#include <vagabond/utils/Eigen/Dense>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>

namespace
{

// minimum-variance/inverse-covariance style redundancy weighting: given a
// symmetric NxN pairwise overlap ("how much do these two subnetworks tell
// us the same thing") matrix, returns one weight per subnetwork such that
// a lone, uncorrelated subnetwork gets weight ~1 and a block of n mutually
// near-identical subnetworks collectively also gets weight ~1 (~1/n each),
// rather than n times the influence of the lone one. w = pinv(overlap) * 1
// - the pseudo-inverse is built from overlap's own SVD (overlap = U Sigma
// V^T) with small singular values floored to zero before inverting, since
// a block of near-duplicate subnetworks is exactly what drives those
// singular values towards zero (near-singular overlap matrix) - Sigma
// itself is only ever used here to regularise that inversion, never as a
// weight in its own right.
Eigen::VectorXf redundancy_weights(const Eigen::MatrixXf &overlap)
{
	int n = (int)overlap.rows();
	if (n <= 1)
	{
		return Eigen::VectorXf::Ones(n);
	}

	Eigen::JacobiSVD<Eigen::MatrixXf> svd(overlap,
	                                      Eigen::ComputeFullU | Eigen::ComputeFullV);
	const Eigen::VectorXf &sv = svd.singularValues();

	float tolerance = sv(0) * 1e-4f;
	Eigen::VectorXf invSv(n);
	for (int i = 0; i < n; i++)
	{
		invSv(i) = (sv(i) > tolerance) ? 1.f / sv(i) : 0.f;
	}

	Eigen::MatrixXf pinv = svd.matrixV() * invSv.asDiagonal() *
	                       svd.matrixU().transpose();

	return pinv * Eigen::VectorXf::Ones(n);
}

}

OccupancyPredictor::OccupancyPredictor(Clique *clique) : _clique(clique)
{

}

OccupancyPredictor::EstimateMap OccupancyPredictor::estimates()
{
	auto timeStart = std::chrono::steady_clock::now();

	struct OccupancyEstimate // one per (ptp, subdivision)
	{
		std::map<int, float> results{}; // one per state
		float sum = 0;
		size_t samples = 0;

		// the originating Clique's own sampleWeight() (see its own
		// comment) - how many independent Subdivide::subdivide() walks
		// collapsed into this one searched subdivision, so it counts
		// proportionally more towards the pooled estimate below than a
		// subdivision only a single walk ever reached.
		float sampleWeight = 1.f;
	};

	struct SubdivisionInfo
	{
		// every ptp this subdivision has a certain (not -1, i.e.
		// CertainStates::operator()(ptp) found it) value for - this
		// subdivision's own "region", used below to score how much two
		// subdivisions overlap independently of any one target ptp.
		OpSet<ProbeTypePair> certainPtps;

		// only used by the optional propagation pass below.
		const CertainStates *states = nullptr;
		std::vector<float> probs;
	};

	// gathered up front (one pass over every subdivision), before any
	// pooling/refinement happens below - each ptp's own list of
	// subnetworks it was actually sampled (certainly) in.
	std::vector<SubdivisionInfo> subs;
	std::map<ProbeTypePair, std::vector<std::pair<size_t, OccupancyEstimate>>>
	occupancies;

	// split of the gather phase's time, for the log at the end: replaying
	// each state's stored energy closure (probsForLocalAve()), against
	// everything else (proportions and bookkeeping).
	std::chrono::steady_clock::duration scoringTime{};
	std::chrono::steady_clock::duration proportionsTime{};

	for (Clique &clique : _clique->subdivisions())
	{
		if (!clique.states()) continue;
		const CertainStates &states = *clique.states();

		size_t idx = subs.size();
		subs.push_back({});

		// computed once per clique, not per ptp - see proportions()'s own
		// comment for why (score() is a live callback, and every ptp in
		// this loop shares the same ave). probsForLocalAve() also avoids
		// calling score(i) a second time just to get ave, unlike the
		// average_score() + probsForAve(ave) pair this used to be.
		float ave = 0;
		auto scoreStart = std::chrono::steady_clock::now();
		std::vector<float> probs = states.probsForLocalAve(ave);
		auto scoreEnd = std::chrono::steady_clock::now();
		scoringTime += scoreEnd - scoreStart;

		subs[idx].states = &states;
		subs[idx].probs = probs;

		// every probe's proportions in one pass over the states (see
		// allProportions()'s own comment) - same numbers proportions() gave
		// one ptp at a time, without a cache miss per access. Their shared
		// total is just the sum of the state weights, as proportions()'s
		// own `sum` was.
		std::vector<std::map<int, float>> allOccs =
		states.allProportions(probs);
		float sum = 0;
		for (const float &prob : probs)
		{
			sum += prob;
		}
		float sampleWeight = (float)clique.sampleWeight();

		for (const ProbeTypePair &ptp : states.ptps())
		{
			int row = states(ptp);
			if (row == -1)
			{
				continue; // not certain in this subdivision
			}

			// only the redundancy weighting reads this
			if (_redundancyWeightingEnabled)
			{
				subs[idx].certainPtps.insert(ptp);
			}

			// the node-sample-count correction (see
			// CertainStates::sampleCount()) is applied inside proportions()
			// itself, not here - sampleWeight is the separate, independent
			// subdivision-merge correction (Clique::sampleWeight()).
			occupancies[ptp].push_back({idx, {allOccs[row], sum,
			                            states.state_count(), sampleWeight}});
		}

		proportionsTime += std::chrono::steady_clock::now() - scoreEnd;
	}

	// pooled occupancy of each existence ptp before any propagation, kept
	// only so the summary log at the end can show what propagation changed.
	auto timeGathered = std::chrono::steady_clock::now();
	std::map<ProbeTypePair, float> uncorrected;

	// why an atom's occupancy did or didn't move under propagation,
	// aggregated over every subnetwork it is in - see the log at the end.
	struct AtomDiagnostic
	{
		int subnetworks = 0;
		float maxCorrelation = 0;
		float maxDeviation = 0;
		int usefulReferences = 0;
		int capped = 0;
	};
	std::map<ProbeTypePair, AtomDiagnostic> atomDiagnostics;

	// optional: correct each atom's occupancy within each subnetwork using
	// every other atom of that subnetwork at once (see OccupancyPropagation)
	// - an atom sampled in only a few, unrepresentative subnetworks is
	// pulled towards what correlated, better-sampled atoms say. Done on the
	// per-subnetwork results themselves, so everything below (union-find,
	// pooling, redundancy weights) runs unchanged on the corrected values.
	if (_propagationEnabled)
	{
		// each existence ptp's pooled ("global") occupancy, computed exactly
		// as the pooling further down does for state 2 over states 1+2
		// (weight est.sum, quantity = that state's total) - so a reference
		// atom's global value here is the very number it would be reported
		// with.
		auto state_total = [](const OccupancyEstimate &est, int state)
		{
			auto it = est.results.find(state);
			return (it == est.results.end()) ? 0.f : it->second;
		};

		std::map<ProbeTypePair, int> atomIndex;
		std::vector<float> thetas;

		for (auto &pair : occupancies)
		{
			if (pair.first.second != hnet::Types::ExistenceType) continue;

			float num = 0, den = 0;
			for (auto &entry : pair.second)
			{
				const OccupancyEstimate &est = entry.second;
				float t1 = state_total(est, 1);
				float t2 = state_total(est, 2);
				num += est.sum * t2;
				den += est.sum * (t1 + t2);
			}

			if (!(den > 0)) continue;

			atomIndex[pair.first] = (int)thetas.size();
			thetas.push_back(num / den);
			uncorrected[pair.first] = num / den;
		}

		struct Member
		{
			ProbeTypePair ptp;
			OccupancyEstimate *est;
			int atom;
		};

		std::vector<std::vector<Member>> members(subs.size());
		for (auto &pair : occupancies)
		{
			auto found = atomIndex.find(pair.first);
			if (found == atomIndex.end()) continue;

			for (auto &entry : pair.second)
			{
				OccupancyEstimate &est = entry.second;
				if (!(state_total(est, 1) + state_total(est, 2) > 0)) continue;

				members[entry.first].push_back({pair.first, &est,
				                                found->second});
			}
		}

		// builds one subnetwork's input on demand. Only ever one of these
		// exists at a time (it is rebuilt for each of the two passes below):
		// holding every subnetwork's states x atoms matrix at once is
		// gigabytes on a big network.
		//
		// Only the highest-weight states are kept - those holding 99% of
		// the subnetwork's Boltzmann weight, at most 512 - since the rest
		// contribute nothing to a weighted correlation but cost states x
		// atoms^2 to include.
		const size_t maxStates = 512;
		auto build = [&](size_t idx, OccupancyPropagation::Subnetwork &in)
		{
			const SubdivisionInfo &info = subs[idx];
			size_t k = members[idx].size();
			if (k < 2 || !info.states) return false;

			std::vector<int> order(info.probs.size());
			for (size_t s = 0; s < order.size(); s++) order[s] = (int)s;
			std::sort(order.begin(), order.end(), [&](int a, int b)
			{
				return info.probs[a] > info.probs[b];
			});

			float total = 0;
			for (float p : info.probs) total += p;

			size_t keep = 0;
			float covered = 0;
			while (keep < order.size() && keep < maxStates &&
			       covered < 0.99f * total)
			{
				covered += info.probs[order[keep]];
				keep++;
			}
			if (keep == 0) return false;

			in.ids.clear();
			in.X = Eigen::MatrixXf((int)keep, (int)k);
			in.recorded = Eigen::MatrixXf::Zero((int)keep, (int)k);
			in.p = Eigen::VectorXf((int)keep);
			in.fraction = Eigen::VectorXf((int)k);
			in.mass = members[idx][0].est->sum;

			for (size_t s = 0; s < keep; s++)
			{
				in.p((int)s) = info.probs[order[s]];
			}

			for (size_t c = 0; c < k; c++)
			{
				const Member &member = members[idx][c];
				float t1 = state_total(*member.est, 1);
				float t2 = state_total(*member.est, 2);
				float f = t2 / (t1 + t2);
				int row = (*info.states)(member.ptp);

				for (size_t s = 0; s < keep; s++)
				{
					// 2 = present, 1 = absent (hnet::Existence); a state
					// that recorded neither leaves this atom unassigned
					// there, masked out of every correlation involving it.
					int v = info.states->value(order[s], row);
					bool known = (v == 1 || v == 2);
					in.X((int)s, (int)c) = (v == 2) ? 1.f : 0.f;
					in.recorded((int)s, (int)c) = known ? 1.f : 0.f;
				}

				in.ids.push_back(member.atom);
				in.fraction((int)c) = f;
			}

			return true;
		};

		// each atom's own total support: samples summed over every
		// subnetwork it is in.
		std::map<ProbeTypePair, float> support;
		for (auto &pair : occupancies)
		{
			float total = 0;
			for (auto &entry : pair.second)
			{
				total += (float)entry.second.samples;
			}
			support[pair.first] = total;
		}

		OccupancyPropagation::Propagator propagator;
		OccupancyPropagation::Subnetwork in;

		// pass 1: pool correlations across every subnetwork.
		for (size_t idx = 0; idx < subs.size(); idx++)
		{
			if (build(idx, in))
			{
				propagator.accumulate(in);
			}
			else
			{
				// still counts towards the average subnetwork mass.
				OccupancyPropagation::Subnetwork empty;
				empty.mass = members[idx].size() ?
				members[idx][0].est->sum : 0.f;
				propagator.accumulate(empty);
			}
		}

		// pass 2: correct each subnetwork, writing straight back into its
		// per-state results (the pooling below then runs unchanged).
		Eigen::VectorXf theta = Eigen::Map<Eigen::VectorXf>(thetas.data(),
		                                                    thetas.size());
		std::vector<OccupancyPropagation::Diagnostic> diagnostics;

		// ROPE_TRACE_ATOM=<text>: for every atom whose desc() contains this,
		// print each subnetwork's local value, corrected value and the
		// references that moved it - to see why an atom's occupancy changed.
		const char *trace = std::getenv("ROPE_TRACE_ATOM");

		for (size_t idx = 0; idx < subs.size(); idx++)
		{
			if (!build(idx, in)) continue;

			Eigen::VectorXf corrected = propagator.correct(in, theta,
			                                               &diagnostics);

			for (size_t c = 0; c < members[idx].size(); c++)
			{
				OccupancyEstimate &est = *members[idx][c].est;
				float mass = state_total(est, 1) + state_total(est, 2);
				float local = mass > 0 ? state_total(est, 2) / mass : 0.f;

				// scale the correction by how little support this atom has
				// of its own (see setPropagationSupport()).
				float lambda = _propagationSupport /
				(support.at(members[idx][c].ptp) + _propagationSupport);
				float f = local + lambda * (corrected((int)c) - local);

				est.results[2] = mass * f;
				est.results[1] = mass * (1.f - f);

				if (trace)
				{
					std::string name = members[idx][c].ptp.first->desc();
					if (name.find(trace) != std::string::npos)
					{
						std::cout << "[trace] " << name << " subnetwork " << idx
						          << " (weight " << est.sum << "): local "
						          << local << " -> " << f << ";";
						for (const auto &ref : diagnostics[c].topReferences)
						{
							std::cout << " [" << members[idx][ref.index]
							.ptp.first->desc() << " corr " << ref.correlation
							          << " slope " << ref.slope << " deviation "
							          << ref.deviation << "]";
						}
						std::cout << std::endl;
					}
				}

				AtomDiagnostic &ad = atomDiagnostics[members[idx][c].ptp];
				ad.subnetworks++;
				ad.maxCorrelation = std::max(ad.maxCorrelation,
				                             diagnostics[c].maxCorrelation);
				ad.maxDeviation = std::max(ad.maxDeviation,
				                           diagnostics[c].maxDeviation);
				ad.usefulReferences += diagnostics[c].usefulReferences;
				ad.capped += diagnostics[c].capped ? 1 : 0;
			}
		}
	}

	auto timePropagated = std::chrono::steady_clock::now();

	// restrict to one atom per mutualExistenceNeighbours() group (their
	// existence is unambiguously tied together, see its own comment on
	// Probe.h, so they'd otherwise report duplicate/redundant occupancy
	// estimates for what is really one shared answer). Transitive closure
	// via union-find over every ExistenceType ptp's own probe, scoped to
	// probes we actually have estimates for.
	std::map<Probe *, Probe *> parent;
	std::function<Probe *(Probe *)> find = [&](Probe *p) -> Probe *
	{
		if (parent.at(p) != p)
		{
			parent[p] = find(parent.at(p));
		}
		return parent.at(p);
	};
	auto unite = [&](Probe *a, Probe *b)
	{
		Probe *ra = find(a), *rb = find(b);
		if (ra != rb)
		{
			parent[ra] = rb;
		}
	};

	// TEMPORARILY DISABLED (diagnosing unexpected atom loss) - every probe
	// stays its own singleton group below, so the representative filter
	// later on is a no-op and nothing gets dropped by dedup. Re-enable by
	// restoring the unite() call once the rest of the pipeline is
	// confirmed OK on its own.
	const bool unionisationEnabled = false;

	for (auto &pair : occupancies)
	{
		if (pair.first.second != hnet::Types::ExistenceType) continue;

		Probe *p = pair.first.first;
		if (parent.count(p) == 0) parent[p] = p;

		if (!unionisationEnabled) continue;

		for (Probe *other : p->mutualExistenceNeighbours())
		{
			if (parent.count(other) == 0) parent[other] = other;

			// never merge across a residue boundary - taking the full
			// transitive closure unrestricted chains an entire alt-conf
			// stretch of backbone into one group (see
			// Network::makeCertainCovalentBond()'s own comment on
			// mutualExistenceNeighbours() risking exactly this), losing
			// every atom in it but one representative. Closure is still
			// taken in full within a single residue's own atoms.
			bool sameResidue = p->atom() && other->atom() &&
			p->atom()->chain() == other->atom()->chain() &&
			p->atom()->residueId() == other->atom()->residueId();

			if (sameResidue)
			{
				unite(p, other);
			}
		}
	}

	// one representative per group: whichever member is a reporter atom
	// (Atom::isReporterAtom()), falling back to whichever member of the
	// group was encountered first if none qualifies.
	std::map<Probe *, Probe *> representative;
	for (auto &pair : occupancies)
	{
		if (pair.first.second != hnet::Types::ExistenceType) continue;

		Probe *p = pair.first.first;
		Probe *root = find(p);

		auto it = representative.find(root);
		bool reporter = p->atom() && p->atom()->isReporterAtom();

		if (it == representative.end())
		{
			representative[root] = p;
		}
		else
		{
			bool curReporter = it->second->atom() &&
			it->second->atom()->isReporterAtom();
			if (reporter && !curReporter)
			{
				representative[root] = p;
			}
		}
	}

	// pairwise overlap between every pair of subdivisions - shared/(shared
	// + unique) of their certainPtps sets - independent of any one target
	// ptp, so computed once up front and sliced per ptp below.
	//
	// only built when redundancy weighting is on - it is O(n^2 * atoms) over
	// the n subnetworks (each pair a set intersection), and nothing reads it
	// otherwise. It used to be built unconditionally, which made every
	// estimates() call (57 of them in a pH scan) pay for it regardless.
	size_t n = _redundancyWeightingEnabled ? subs.size() : 0;
	Eigen::MatrixXf overlap = Eigen::MatrixXf::Identity((int)n, (int)n);

	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = i + 1; j < n; j++)
		{
			const OpSet<ProbeTypePair> &A = subs[i].certainPtps;
			const OpSet<ProbeTypePair> &B = subs[j].certainPtps;

			size_t shared = 0;
			for (const ProbeTypePair &ptp : A)
			{
				if (B.count(ptp)) shared++;
			}

			size_t total = A.size() + B.size() - shared;
			float ov = (total > 0) ? (float)shared / (float)total : 0.f;
			overlap((int)i, (int)j) = overlap((int)j, (int)i) = ov;
		}
	}

	EstimateMap ret;

	for (auto &occs : occupancies)
	{
		const ProbeTypePair &ptp = occs.first;
		if (ptp.second != hnet::Types::ExistenceType)
		{
			continue; // skip for now
		}

		if (representative.at(find(ptp.first)) != ptp.first)
		{
			continue; // not this group's chosen representative
		}

		const auto &estimates = occs.second;
		size_t m = estimates.size();

		// this ptp's own subnetworks, sliced out of the global overlap
		// matrix, then de-weighted by redundancy (see redundancy_weights()'
		// own comment) - a lone subnetwork keeps its full contribution, a
		// cluster of near-identical ones shares it between them.
		Eigen::VectorXf redundancy = Eigen::VectorXf::Ones((int)m);
		if (_redundancyWeightingEnabled)
		{
			Eigen::MatrixXf sub((int)m, (int)m);
			for (size_t a = 0; a < m; a++)
			{
				for (size_t b = 0; b < m; b++)
				{
					sub((int)a, (int)b) = overlap((int)estimates[a].first,
					                              (int)estimates[b].first);
				}
			}
			redundancy = redundancy_weights(sub);
		}

		std::map<int, float> sums;
		float grand_sum = 0;
		size_t sample_count = 0;

		for (const int &state : {1, 2})
		{
			float sum = 0; float weights = 0;
			for (size_t a = 0; a < m; a++)
			{
				const OccupancyEstimate &est = estimates[a].second;
				if (est.results.count(state) == 0)
				{
					continue;
				}

				// TEMPORARILY DISABLED (diagnosing atom loss/weighting
				// behaviour alongside unionisationEnabled above) -
				// est.sampleWeight left out of the product for now.
				float weight = est.sum * redundancy((int)a);
				float quantity = est.results.at(state);
				sample_count += est.samples;
				sum += quantity * weight;
				weights += weight;
			}
			if (sum != sum)
			{
				sum = 0;
			}
			sum /= weights;
			sums[state] = sum;
			grand_sum += sum;
		}

		for (auto &s : sums)
		{
			s.second /= grand_sum;
		}

		if (!ptp.first->atom() || ptp.first->atom()->symmetryCopyOf()
		    || ptp.first->is_bulk())
		{
			continue;
		}

		Atom *atom = ptp.first->atom();

		if (atom->bondLengthCount() && atom->occupancy_sum() < 0.9)
		{
			continue;
		}

		if (!ptp.first->isActiveAtom())
		{
			continue;
		}

		float calculated = sums[2] / (sums[1] + sums[2]);
		if (calculated != calculated)
		{
			calculated = 0;
			ret.erase(ptp);
			continue;
		}
		float observed = ptp.first->atomConf().occupancy();

		ret[ptp] = {calculated, observed, grand_sum, sample_count};
	}

	if (_propagationEnabled)
	{
		struct Shift
		{
			const ProbeTypePair *ptp;
			float before;
			float after;
			size_t samples;
		};

		std::vector<Shift> shifts;
		float total = 0;
		for (auto &pair : ret)
		{
			auto it = uncorrected.find(pair.first);
			if (it == uncorrected.end()) continue;

			float after = pair.second.calculated;
			total += fabs(after - it->second);
			shifts.push_back({&pair.first, it->second, after,
			                  pair.second.samples});
		}

		std::sort(shifts.begin(), shifts.end(),
		          [](const Shift &a, const Shift &b)
		          {
			          return fabs(a.after - a.before) >
			          fabs(b.after - b.before);
		          });

		std::cout << "OccupancyPredictor: propagation on, " << shifts.size()
		          << " atoms, mean |shift| = "
		          << (shifts.size() ? total / shifts.size() : 0.f)
		          << "; largest shifts (pooled before -> after, samples):"
		          << std::endl;

		for (size_t i = 0; i < shifts.size() && i < 15; i++)
		{
			std::cout << "  " << shifts[i].ptp->first->desc() << ": "
			          << shifts[i].before << " -> " << shifts[i].after
			          << " (" << shifts[i].samples << ")" << std::endl;
		}

		// atoms sitting near 50% that propagation left essentially where
		// they were - and why, from what each of their subnetworks saw:
		//  alone      - never in a subnetwork with another existence atom
		//  uncorrel.  - no other atom there correlated with it (>= 0.1)
		//  agreeing   - correlated atoms there, but all already at their
		//               global values, so nothing to transfer
		//  other      - had correlated, disagreeing atoms and still didn't
		//               move (e.g. regression cancelled out)
		// does propagation push occupancies away from 50% (more extreme) or
		// towards it, overall? Positive mean = more extreme on average.
		{
			int more = 0, less = 0;
			double change = 0;
			for (const Shift &shift : shifts)
			{
				float delta = fabs(shift.after - 0.5f) -
				fabs(shift.before - 0.5f);
				change += delta;
				if (delta > 0.001f) more++;
				else if (delta < -0.001f) less++;
			}

			std::cout << "OccupancyPredictor: " << more << " atoms more "
			          << "extreme, " << less << " less extreme, "
			          << (shifts.size() - more - less) << " unchanged; mean "
			          << "change in distance from 50% = "
			          << (shifts.size() ? change / shifts.size() : 0.0)
			          << std::endl;
		}

		int stuck = 0, alone = 0, uncorrelated = 0, agreeing = 0, other = 0;
		std::vector<std::string> examples;
		for (const Shift &shift : shifts)
		{
			if (fabs(shift.after - shift.before) > 0.005f ||
			    fabs(shift.before - 0.5f) > 0.1f)
			{
				continue;
			}

			stuck++;
			auto it = atomDiagnostics.find(*shift.ptp);
			std::string reason;

			if (it == atomDiagnostics.end())
			{
				alone++;
				reason = "alone";
			}
			else if (it->second.maxCorrelation < 0.1f)
			{
				uncorrelated++;
				reason = "uncorrel.";
			}
			else if (it->second.usefulReferences == 0)
			{
				agreeing++;
				reason = "agreeing";
			}
			else
			{
				other++;
				reason = "other";
			}

			if (examples.size() < 10)
			{
				std::string line = "  " + shift.ptp->first->desc() + ": " +
				reason;
				if (it != atomDiagnostics.end())
				{
					line += " (subnetworks " +
					std::to_string(it->second.subnetworks) + ", max corr " +
					std::to_string(it->second.maxCorrelation) +
					", max reference deviation " +
					std::to_string(it->second.maxDeviation) + ")";
				}
				examples.push_back(line);
			}
		}

		std::cout << "OccupancyPredictor: " << stuck << " atoms near 50% "
		          << "unmoved - alone " << alone << ", uncorrelated "
		          << uncorrelated << ", correlated atoms agreeing "
		          << agreeing << ", other " << other << std::endl;
		for (const std::string &line : examples)
		{
			std::cout << line << std::endl;
		}
	}

	{
		auto done = std::chrono::steady_clock::now();
		auto ms = [](auto a, auto b)
		{
			return (long)std::chrono::duration_cast<std::chrono::milliseconds>
			(b - a).count();
		};

		if (ms(timeStart, done) > 500)
		{
			std::cout << "OccupancyPredictor::estimates(): "
			          << ms(timeStart, done) << " ms total (gather "
			          << ms(timeStart, timeGathered) << " [scoring "
			          << std::chrono::duration_cast<std::chrono::milliseconds>
			          (scoringTime).count() << ", proportions "
			          << std::chrono::duration_cast<std::chrono::milliseconds>
			          (proportionsTime).count() << "], propagation "
			          << ms(timeGathered, timePropagated) << ", rest "
			          << ms(timePropagated, done) << "), "
			          << subs.size() << " subnetworks" << std::endl;
		}
	}

	return ret;
}
