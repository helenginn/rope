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

#ifndef __vagabond__CertainStates__
#define __vagabond__CertainStates__

#include <cmath>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/OpSet.h>
#include "ProbeResult.h"

int dim_for_type(const hnet::Types &type);

struct ProbeCorrelation
{
	ProbeTypePair left{};
	ProbeTypePair right{};
	Eigen::MatrixXf mat{};
};

class CertainStates
{
public:
	CertainStates() {}
	CertainStates(const std::vector<ProbeResult> &results);

	int operator()(const ProbeTypePair &ptp) const;
	
	const OpSet<ProbeTypePair> &ptps() const
	{
		return _headers;
	}
	
	size_t state_count() const
	{
		return _data.cols();
	}
	
	size_t probe_count() const
	{
		return _data.rows();
	}
	
	int value(int r, int p) const; // indices for state, probe
	float score(int r) const; // indices for state
	
	float average_score() const;

	// score(i) is a live callback (see GetScore in ProbeResult.h) that can
	// change value if the enabled energy terms change between calls -
	// probsForAve() must therefore be recomputed by the caller at the
	// start of each correlation run, never cached across separate runs.
	// Callers loop correlate() O(ptps^2) times per run, all sharing the
	// same ave, so computing this vector once up front and passing it in
	// (rather than recomputing exp() per state inside every correlate()
	// call) avoids a large amount of redundant work without risking
	// staleness - it is fresh for exactly as long as the run that made it.
	std::vector<float> probsForAve(float ave) const;

	// for callers that need both the average and the probs vector for
	// the same (local, this-clique-only) ave, e.g. OccupanciesView,
	// ViewCorrelations - average_score() followed by probsForAve(ave)
	// would call the live score(i) callback for every state twice over
	// (once to sum for the average, again inside probsForAve()). This
	// calls it once per state, fills ave by reference from that same
	// pass, and derives probs from the cached values instead of calling
	// score(i) again. Not a fit for Correlative, which needs probs for
	// a *global* ave pooled across many cliques rather than each
	// clique's own local one - see probsForAve() there.
	std::vector<float> probsForLocalAve(float &ave) const;

	ProbeCorrelation correlate(const ProbeTypePair &left,
	                           const ProbeTypePair &right,
	                           const std::vector<float> &probs,
	                           bool norm = true) const;

	// takes a pre-computed probs vector (see probsForAve() above) rather
	// than ave directly, for the same reason correlate() does - callers
	// looping over ptps() all share the same ave, so computing this once
	// up front avoids redoing an O(state_count) pass (including a call
	// to the potentially-expensive live score() callback per state) on
	// every single call.
	std::map<int, float> proportions(ProbeTypePair ptp, float &sum,
	                                 const std::vector<float> &probs) const;
private:
	// a set, not a vector, so header (row) order is a deterministic
	// function of which probes are present - not of the order they
	// happened to be discovered while scanning results (which could vary
	// run to run - see ExhaustiveSearch's _wider). _lookup's indices are
	// assigned in a dedicated pass over the finished, fully-sorted set
	// (see the constructor) - not during insertion, since a set's sorted
	// position for existing elements can shift as later elements are
	// added.
	OpSet<ProbeTypePair> _headers;
	std::map<ProbeTypePair, int> _lookup;

	Eigen::MatrixXi _data;
//	Eigen::VectorXf _scores;

	std::vector<GetScore> _scores;
};

#endif
