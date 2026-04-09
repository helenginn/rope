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

#ifndef __vagabond__ProbeResult__
#define __vagabond__ProbeResult__

#include <vagabond/core/protonic/hnet.h>
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/protonic/Probe.h>

using Eigen::MatrixXf;

class Probe;

struct ProbeTypePair : public std::pair<Probe *, hnet::Types>
{
	ProbeTypePair(const std::pair<Probe *, hnet::Types> &orig = {})
	{
		this->first = orig.first;
		this->second = orig.second;
	}

	bool operator<(const ProbeTypePair &other) const
	{
		if (!this->first || !other.first)
		{
			return this->first < other.first;
		}
		else if (this->second == other.second)
		{
			return this->first->desc() < other.first->desc();
		}
		else
		{
			return this->second > other.second;
		}
	}
};

struct OneProbe
{
	Probe *probe;
	hnet::Types type;
	int value;
};

struct ProbeResult
{
	std::vector<OneProbe> results;
	float score;
};

struct ProbeCorrelation
{
	ProbeTypePair left{};
	ProbeTypePair right{};
	hnet::Types lType{};
	hnet::Types rType{};
	MatrixXf mat{};
};

int dim_for_type(const hnet::Types &type);

std::vector<ProbeTypePair> probes(const std::vector<ProbeResult> &source);
float average_score(const std::vector<ProbeResult> &source);

ProbeCorrelation correlate(const std::vector<ProbeResult> &source, 
                           ProbeTypePair left, ProbeTypePair right, 
                           float all_ave, bool norm);

#endif
