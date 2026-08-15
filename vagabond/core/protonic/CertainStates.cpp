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

#include "CertainStates.h"
#include "ProbeResult.h"

int dim_for_type(const hnet::Types &type)
{
	return (type == hnet::Types::BondType ? 3 : 2);
}

CertainStates::CertainStates(const std::vector<ProbeResult> &results)
{
	// row = one probe's states
	// cols = number of H-bonding states
	int cols = results.size();
	if (cols == 0)
	{
		return;
	}

	// first pass: union of every probe+type pair seen across all results,
	// not just results[0] - different results can carry different sets
	// of wider-clique probes driven to certainty, so no single result is
	// guaranteed to list everything the others do. Just insert here -
	// _headers being a set means row order falls out of its own sort
	// order, not insertion order, so _lookup can't be assigned yet: a
	// later insertion can sort before an earlier one, which would shift
	// every index assigned so far.
	for (const ProbeResult &result : results)
	{
		for (const OneProbe &op : result.results())
		{
			_headers.insert(ProbeTypePair({op.probe, op.type}));
		}
	}

	// now that the set is complete and its order fixed, walk it once to
	// assign each row its final, stable index.
	int row_index = 0;
	for (const ProbeTypePair &ptp : _headers)
	{
		_lookup[ptp] = row_index;
		row_index++;
	}

	int rows = _headers.size();
	_data = Eigen::MatrixXi(rows, cols);
	_data.setConstant(-1);
	_scores.resize(cols);

	// second pass: fill in whatever each result actually recorded: a
	// probe missing from a particular result (never decreed there and
	// never became certain as a side effect) is left as -1 - value()
	// returns that as-is, and correlate() skips it.
	for (int j = 0; j < cols; j++)
	{
		_scores[j] = results[j].getScore();

		for (const OneProbe &op : results[j].results())
		{
			ProbeTypePair ptp = ProbeTypePair({op.probe, op.type});
			int row = _lookup.at(ptp);
			_data(row, j) = op.value;
		}
	}
}

int CertainStates::operator()(const ProbeTypePair &ptp) const
{
	if (_lookup.count(ptp) == 0) return -1;

	return _lookup.at(ptp);
}

int CertainStates::value(int col, int row) const
{
	return _data(row, col);
}

float CertainStates::score(int col) const
{
	return _scores[col]();
}

float CertainStates::average_score() const
{
	float sum = 0;
	for (const GetScore &score : _scores)
	{
		sum += score();
	}
	return sum / (float)state_count();
}

int CertainStates::distance(int stateA, int stateB) const
{
	int changes = 0;

	for (int row = 0; row < (int)probe_count(); row++)
	{
		int a = value(stateA, row);
		int b = value(stateB, row);

		if (a < 0 || b < 0)
		{
			continue;
		}

		if (a != b)
		{
			changes++;
		}
	}

	return changes;
}

Eigen::MatrixXi CertainStates::distanceMatrix() const
{
	int n = (int)state_count();
	Eigen::MatrixXi dist = Eigen::MatrixXi::Zero(n, n);

	for (int i = 0; i < n; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			int d = distance(i, j);
			dist(i, j) = d;
			dist(j, i) = d;
		}
	}

	return dist;
}

std::vector<float> CertainStates::probsForAve(float ave) const
{
	float rt = 2.57;
	std::vector<float> probs(state_count());
	for (size_t i = 0; i < state_count(); i++)
	{
		probs[i] = exp((ave - score(i)) / rt);
	}

	return probs;
}

std::vector<float> CertainStates::probsForLocalAve(float &ave) const
{
	float rt = 2.57;
	std::vector<float> scores(state_count());
	float sum = 0;

	for (size_t i = 0; i < state_count(); i++)
	{
		scores[i] = score(i);
		sum += scores[i];
	}

	ave = sum / (float)state_count();

	std::vector<float> probs(state_count());
	for (size_t i = 0; i < state_count(); i++)
	{
		probs[i] = exp((ave - scores[i]) / rt);
	}

	return probs;
}

ProbeCorrelation CertainStates::correlate(const ProbeTypePair &left,
                                          const ProbeTypePair &right,
                                          const std::vector<float> &probs,
                                          bool relative) const
{
	if (state_count() == 0 || probe_count() == 0)
	{
		return {};
	}

	ProbeCorrelation corr{};
	corr.left = left;
	corr.right = right;
	
	// we have full list of source material consisting of all nodes, but
	// we only want ccs for a pair of nodes: left and right. 
	// We now find their indices.
	int m = (*this)(left);
	int n = (*this)(right);
	
	// establishing dimensions of this particular matrix
	int rows = dim_for_type(corr.left.second);
	int cols = dim_for_type(corr.right.second);
	
	corr.mat = Eigen::MatrixXf(rows, cols);
	corr.mat.setZero();
	
	// func: based on a value coming out of the probe result, we convert it
	// to an index.
	// only checks bits 0-3: fine for Bond::Values/Existence::Values
	// (ExhaustiveSearch.cpp), but hnet::Count::Values (charge, e.g.
	// Count::mOne = 1 << 16) can set bits well past that - correlate()
	// isn't yet called with a ChargeType pair anywhere, so this hasn't
	// mattered, but it will need widening before it is.
	auto get_index = [](const int &v)
	{
		for (int i = 0; i <= 3; i++)
		{
			if (v & (1 << i))
			{
				return (i % 3);
			}
		}
		return -1;
	};
	
	// first we need to collect averages, by figuring out proportions for each
	// possible result for left and right nodes.
	// we want to keep track of relative probabilities of each state according
	// to partial energy knowledge.
	float total_probs = 0;

	for (int i = 0; i < state_count(); i++)
	{
		int mv = value(i, m);
		int nv = value(i, n);

		// -1 means this particular result never recorded a value for
		// left or right (see the constructor) - nothing to correlate
		// for this state, so it must not contribute to either the
		// histogram or total_probs.

		// we also want to figure out the total sum of energy weights
		float prob = probs[i];

		if (mv == -1 && nv == -1)
		{
			continue;
		}

		total_probs += prob;

		if (mv == -1 || nv == -1)
		{
			continue;
		}

		int l = get_index(mv);
		int r = get_index(nv);

		// for Boltzmann energy calculation
		corr.mat(l, r) += prob;
	}
	
	Eigen::MatrixXf copy = corr.mat;

	for (int j = 0; j < corr.mat.rows(); j++)
	{
		float row_sum = copy.row(j).sum();
		for (int i = 0; i < corr.mat.cols(); i++)
		{
			float outcome_given_row = copy(j, i) / row_sum;

			float col_sum = copy.col(i).sum();
			float average_outcome = col_sum / total_probs;
			float enrichment = outcome_given_row - average_outcome;
//			enrichment *= (copy(j, i) / total_probs);

			corr.mat(j, i) = enrichment;
		}
	}

	for (int i = 0; i < corr.mat.rows(); i++)
	{
		for (int j = 0; j < corr.mat.cols(); j++)
		{
			if (corr.mat(i, j) != corr.mat(i, j) || !isfinite(corr.mat(i, j)))
			{
				corr.mat(i, j) = 0;
			}
		}
	}

	return corr;
}

std::map<int, float> CertainStates::proportions(ProbeTypePair ptp,
                                                float &sum,
                                                const std::vector<float> &probs) const
{
	if (state_count() == 0 || probe_count() == 0)
	{
		return {};
	}

	int n = (*this)(ptp);

	std::map<int, float> totals;
	std::map<int, int> counts;
	sum = 0;

	for (int i = 0; i < state_count(); i++)
	{
		int nv = value(i, n);
		float contrib = probs[i];
		totals[nv] += contrib;
		counts[nv]++;
		sum += contrib;

	}

	if (false && ptp.first->atom() && ptp.first->atom()->residueId() == 74 && false)
	{
		for (const ProbeTypePair &other : _headers)
		{
			std::cout << other << " ";
		}
		std::cout << std::endl;
		std::cout << "Totals: " << totals[1] << " (" << counts[1] << 
		", absent) " << totals[2] << " (" << counts[2] << 
		", present)" << std::endl;
	}
	
	return totals;
}
