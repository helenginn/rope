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
	int rows = 0;
	if (cols > 0)
	{
		rows = results[0].results().size();
	}
	else
	{
		return;
	}
	
	_headers.reserve(rows);
	for (const OneProbe &op : results[0].results())
	{
		ProbeTypePair ptp = ProbeTypePair({op.probe, op.type});
		_lookup[ptp] = _headers.size();
		_headers.push_back(ptp);
	}

	_data = Eigen::MatrixXi(rows, cols);
	_scores.resize(cols);
	
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			_data(i, j) = results[j].results()[i].value;
			if (i == 0)
			{
				_scores[j] = results[j].getScore();
			}
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

ProbeCorrelation CertainStates::correlate(const ProbeTypePair &left,
                                          const ProbeTypePair &right, 
                                          float ave, bool relative) const
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
	float total_probs = 0;

	for (int i = 0; i < state_count(); i++)
	{
		int mv = value(i, m);
		int nv = value(i, n);
		float sc = score(i);
		
		// we also want to figure out the total sum of energy weights
		float prob = exp((ave - sc) / rt);
		
		if (prob != prob)
		{
			std::cout << "Ave: " << ave << std::endl;
			std::cout << "Score: " << sc << std::endl;
			std::cout << "Prob: " << prob << std::endl;
		}

		total_probs += prob;

		int l = get_index(mv);
		int r = get_index(nv);

		// for Boltzmann energy calculation
		corr.mat(l, r) += prob;
	}
	std::cout << std::endl;
	
	for (int j = 0; j < corr.mat.rows(); j++)
	{
		float row_total = corr.mat.row(j).sum();
		int col_count = corr.mat.cols();
		
		if (row_total > 1e-6 && relative)
		{
			corr.mat.row(j) /= row_total;
			
			for (int i = 0; i < col_count; i++)
			{
				corr.mat.row(j)(i) -= 1.f / (float)col_count;
			}
		}
		else if (!relative)
		{
			corr.mat.row(j) /= total_probs;
		}
	}

	bool done = false;
	for (int i = 0; i < corr.mat.rows() && !done; i++)
	{
		for (int j = 0; j < corr.mat.cols() && !done; j++)
		{
			if (corr.mat(i, j) > 1)
			{
				std::cout << "over one: " << std::endl;
				std::cout << corr.mat << std::endl;
				done = true;
				break;
			}
		}
	}

	return corr;
}

std::map<int, float> CertainStates::proportions(ProbeTypePair ptp, 
                                                float &sum, float ave) const
{
	if (state_count() == 0 || probe_count() == 0)
	{
		return {};
	}

	int n = (*this)(ptp);

	float rt = 2.57;
	std::map<int, float> totals;
	std::map<int, int> counts;
	sum = 0;

	for (int i = 0; i < state_count(); i++)
	{
		int nv = value(i, n);
		float sc = score(i);
		float contrib = exp((ave - sc) / rt);
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
