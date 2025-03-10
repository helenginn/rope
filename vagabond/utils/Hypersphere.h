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

#ifndef __vagabond__Hypersphere__
#define __vagabond__Hypersphere__

#include "ProbDist.h"
#include <vector>

class Hypersphere
{
public:
	/** Generate new hyperspherical set of points
	 *  @param dims number of dimensions of the hypersphere
	 *  @param n number of points to generate */
	Hypersphere(int dims, int n);

	/** call this before retrieving points of Fibonacci lattice */
	void prepareFibonacci();

	/** how many points in total do we have (may not be exactly same as
	 * entry */
	int count() const
	{
		return _points.size();
	}
	
	const std::vector<float> &point(int idx) const
	{
		return _points[idx];
	}
	
	std::vector<float> scaled_point(int idx, float scale) const;
private:
	void setup();
	void setup_distribution_for_dim(int dim);
	ProbDist setup_distribution_for_other(int dim);
	ProbDist setup_distribution_for_first();
	int find_next_prime(std::vector<int> &prev);
	std::vector<float> angles_to_cartesian(std::vector<float> angles);
	
	void preparePrimes();

	int _dims = 3;
	int _n = 0;
	std::vector<int> _primes;
	std::vector<std::vector<float> > _points;

	std::map<int, ProbDist> _distributions;
};

#endif
