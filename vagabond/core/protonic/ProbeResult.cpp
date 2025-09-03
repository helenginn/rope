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

ProbeCorrelation correlate(const std::vector<ProbeResult> &source, 
                           ProbeTypePair left, ProbeTypePair right, bool norm)
{
	if (source.size() == 0)
	{
		return {};
	}

	ProbeCorrelation corr{};
	corr.left = left;
	corr.right = right;
	
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
	
	int rows = dim_for_type(corr.lType);
	int cols = dim_for_type(corr.rType);
	
	corr.mat = MatrixXf(rows, cols);
//	std::cout << corr.mat << std::endl;
	
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
	
	auto print = [](const ProbeResult &pr)
	{
		for (const OneProbe &op : pr.results)
		{
			//std::cout << op.value << "-";
		}
		//std::cout << std::endl;
	};
	
	for (const ProbeResult &pr : source)
	{
		int mv = pr.results[m].value;
		int nv = pr.results[n].value;
		int l = get_index(mv);
		int r = get_index(nv);
		float probability = pr.score;
		
		corr.mat(l, r) += exp(probability);
	}

	if (norm)
	{
		float weight = (float)source.size();
		corr.mat /= weight;
	}
	
	return corr;
}

std::vector<std::pair<Probe *, hnet::Types>> 
probes(const std::vector<ProbeResult> &source)
{
	if (source.size() == 0)
	{
		return {};
	}

	std::vector<std::pair<Probe *, hnet::Types>> ret;
	for (const OneProbe &op : source[0].results)
	{
		ret.push_back({op.probe, op.type});
	}
	
	return ret;
}
