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
#include <vagabond/utils/Eigen/Dense>
#include <functional>

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
	};

	// gathered up front (one pass over every subdivision), before any
	// pooling/refinement happens below - each ptp's own list of
	// subnetworks it was actually sampled (certainly) in.
	std::vector<SubdivisionInfo> subs;
	std::map<ProbeTypePair, std::vector<std::pair<size_t, OccupancyEstimate>>>
	occupancies;

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
		std::vector<float> probs = states.probsForLocalAve(ave);

		for (const ProbeTypePair &ptp : states.ptps())
		{
			if (states(ptp) == -1)
			{
				continue; // not certain in this subdivision
			}

			subs[idx].certainPtps.insert(ptp);

			// the node-sample-count correction (see
			// CertainStates::sampleCount()) is applied inside proportions()
			// itself, not here - sampleWeight is the separate, independent
			// subdivision-merge correction (Clique::sampleWeight()).
			float sum = 0; // sum of all energy contributions, populated next
			std::map<int, float> occs = states.proportions(ptp, sum, probs);
			float sampleWeight = (float)clique.sampleWeight();

			occupancies[ptp].push_back({idx, {occs, sum, states.state_count(),
			                            sampleWeight}});
		}
	}

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
	size_t n = subs.size();
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
		Eigen::MatrixXf sub((int)m, (int)m);
		for (size_t a = 0; a < m; a++)
		{
			for (size_t b = 0; b < m; b++)
			{
				sub((int)a, (int)b) = overlap((int)estimates[a].first,
				                              (int)estimates[b].first);
			}
		}
		Eigen::VectorXf redundancy = _redundancyWeightingEnabled ?
		redundancy_weights(sub) : Eigen::VectorXf::Ones((int)m);

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

	return ret;
}
