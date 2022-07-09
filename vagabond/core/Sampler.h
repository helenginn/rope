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

#ifndef __vagabond__Sampler__
#define __vagabond__Sampler__

#include <vector>
#include "../utils/svd/PCA.h"

class Sampler
{
public:
	Sampler(int n, int dims);

	void setup();
	float hypersphereVolume(float radius);
	
	void multiplyVec(float *&vec, int num);
	
	int dims()
	{
		return _dims;
	}

	const size_t pointCount() const
	{
		return _points.rows;
	}
private:
	float gamma();
	void populateSamples();
	void establishRadius();
	void convertSamples();

	void addPointWithinRadius(std::vector<float> &point);
	bool incrementPoint(std::vector<float> &point);
	std::vector<float> spawnFirstPoint();

	float _radius;
	float _max;
	int _n;
	int _dims;
	
	std::vector<std::vector<float> > _tmpPoints;
	PCA::Matrix _points{};
};

#endif
