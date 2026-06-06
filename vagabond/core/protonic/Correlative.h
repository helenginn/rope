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

#ifndef __vagabond__Correlative__
#define __vagabond__Correlative__

#include <map>
#include <vagabond/utils/OpSet.h>
#include <vagabond/utils/Eigen/Dense>

class Clique;
class CertainStates;
struct ProbeTypePair;

class Correlative
{
public:
	Correlative(const OpSet<ProbeTypePair> &all, float ave_score,
	            bool relative = true);

	static OpSet<ProbeTypePair> probeTypePairs(const std::list<Clique> &cliques,
	                                           float &all_ave);

	const std::function<std::string(float, float)> &matrixLookup() const
	{
		return _matrixLookup;
	}

	const std::map<ProbeTypePair, std::pair<int, int>> &insertions() const
	{
		return _insertions;
	}

	void addStates(const CertainStates &states);

	Eigen::MatrixXf acquireMatrix();
	
	size_t size()
	{
		return _size;
	}
private:
	OpSet<ProbeTypePair> _probes;

	std::map<ProbeTypePair, std::pair<int, int>> _insertions;
	std::map<int, ProbeTypePair> _lookup;
	
	std::function<std::string(float, float)> _matrixLookup;

	Eigen::MatrixXf _overall{};
	Eigen::MatrixXf _written{};
	
	size_t _size = 0;
	float _ave_score = 0;
	bool _relative = true;
};

#endif
