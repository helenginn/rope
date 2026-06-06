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

#include "ProbeResult.h"

int dim_for_type(const hnet::Types &type)
{
	return (type == hnet::Types::BondType ? 3 : 2);
}

float average_score(const std::vector<ProbeResult> &source)
{
	float ave = 0;
	for (const ProbeResult &pr : source)
	{
		float probability = pr.score;
		ave += probability;
	}

	ave /= source.size();
	return ave;
}

std::map<int, float> state_proportions(const std::vector<ProbeResult> &source, 
                                       ProbeTypePair ptp, float &sum, float ave)
{
	if (source.size() == 0)
	{
		return {};
	}

	int n = 0;
	for (const OneProbe &op : source[0].results)
	{
		if (op.probe == ptp.first && op.type == ptp.second)
		{
			break;
		}
		n++;
	}

	float rt = 2.57;
	std::map<int, float> totals;
	sum = 0;

	for (const ProbeResult &pr : source)
	{
		int nv = pr.results[n].value;
		float contrib = exp((ave - pr.score) / rt);
		totals[nv] += contrib;
		sum += contrib;
	}
	
	for (auto &pair : totals)
	{
//		pair.second /= sum;
	}

	return totals;
}

ProbeCorrelation correlate(const std::vector<ProbeResult> &source, 
                           ProbeTypePair left, ProbeTypePair right, float ave,
                           bool norm)
{
	if (source.size() == 0)
	{
		return {};
	}

	ProbeCorrelation corr{};
	corr.left = left;
	corr.right = right;
	
	// we have full list of source material consisting of all nodes, but
	// we only want ccs for a pair of nodes: left and right. 
	// We now find their indices.

	int m = -1; int n = -1; int i = 0;
	for (const OneProbe &op : source[0].results)
	{
		if (op.probe == left.first && op.type == left.second)
		{
			m = i;
			corr.lType = op.type;
		}
		if (op.probe == right.first && op.type == right.second)
		{
			n = i;
			corr.rType = op.type;
		}

		i++;
	}
	
	// establishing dimensions of this particular matrix
	int rows = dim_for_type(corr.lType);
	int cols = dim_for_type(corr.rType);
	
	corr.mat = MatrixXf(rows, cols);
	corr.mat.setZero();
	
	// func: based on a value coming out of the probe result, we convert it 
	// to an index.
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
	float rt = 2.57;
	int idx = 0;
	std::map<int, float> relProbs;
	float total_probs = 0;

	for (const ProbeResult &pr : source)
	{
		int mv = pr.results[m].value;
		int nv = pr.results[n].value;
		
		// we also want to figure out the total sum of energy weights
		float contrib = exp((ave - pr.score) / rt);
		relProbs[idx] = contrib;
		total_probs += contrib;

		idx++;
	}

	idx = 0;
	for (const ProbeResult &pr : source)
	{
		int mv = pr.results[m].value;
		int nv = pr.results[n].value;
		int l = get_index(mv);
		int r = get_index(nv);

		// for Boltzmann energy calculation
		float prob = relProbs[idx];

		corr.mat(l, r) += prob;
		idx++;
	}
	
	for (int j = 0; j < corr.mat.rows(); j++)
	{
		float row_total = corr.mat.row(j).sum();
		int col_count = corr.mat.cols();
		
		if (row_total > 1e-6)
		{
			corr.mat.row(j) /= row_total;
			
			for (int i = 0; i < col_count; i++)
			{
				corr.mat.row(j)(i) -= 1.f / (float)col_count;
			}
		}

	}

	return corr;
}

std::vector<ProbeTypePair>
probes(const std::vector<ProbeResult> &source)
{
	if (source.size() == 0)
	{
		return {};
	}

	std::vector<ProbeTypePair> ret;
	for (const OneProbe &op : source[0].results)
	{
		ret.push_back(ProbeTypePair({op.probe, op.type}));
	}
	
	return ret;
}
