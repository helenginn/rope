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

#ifndef __vagabond__Target__
#define __vagabond__Target__

#include <vagabond/utils/Vec3s.h>
#include <vagabond/utils/svd/PCA.h>

class TorsionData;
class ClusterSVD;
class Instance;

class Target
{
public:
	Target(int num_axes, TorsionData *group,
	       ClusterSVD *cluster, Instance *ref);
	~Target();

	const std::vector<Floats> &pointsForScore();

	void minMaxComponents(size_t num_axes, std::vector<float> &mins, 
	                      std::vector<float> &maxes);

	void transformCoordinates(std::vector<float> &coord);
private:
	void transformMatrix();

	int _nAxes;
	std::vector<float> _weights;
	
	std::vector<Floats> _points;

	Instance *_reference = nullptr;
	TorsionData *_data = nullptr;
	ClusterSVD *_tCluster = nullptr;
	PCA::Matrix _shift{};
};

#endif
