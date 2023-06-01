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

#ifndef __vagabond__Network__
#define __vagabond__Network__

#include <vector>
#include <vagabond/c4x/ClusterTSNE.h>
#include <vagabond/utils/Mapping.h>
#include <vagabond/utils/Vec3s.h>

#define NETWORK_DIMS (2)

template <typename Type> class Mapped;
template <unsigned int D, typename Type> class Mapping;
template <int N, unsigned int D, typename Type> class Face;

class Entity;
class Instance;
class TorsionCluster;
class ClusterTSNE;
class SpecificNetwork;

class Network
{
public:
	Network(Entity *entity, std::vector<Instance *> &instances);
	
	const static int dims()
	{
		return NETWORK_DIMS;
	}
	
	glm::vec3 point(int idx)
	{
		return _tsne->pointForDisplay(idx);
	}
	
	Mapped<float> *blueprint()
	{
		return _blueprint;
	}
	
	Mapped<float> *blueprint_scalar_copy();
	Mapped<glm::vec3> *blueprint_vec3_copy();
	Mapped<Vec3s> *blueprint_vec3s_copy();
	
	std::vector<float> pointForInstance(Instance *instance);
	bool valid_position(const std::vector<float> &vals);
	
	SpecificNetwork *specificForInstance(Instance *instance);

	void setup();
	void convertToTSNE();
	
	const std::vector<Instance *> &instances() const
	{
		return _instances;
	}
	
	const int &indexForInstance(Instance *inst)
	{
		return _instance2Point[inst];
	}
private:
	void makeCluster();
	void makeBlueprintMapping();
	void preparePoints();
	void addBounds(Mapping<NETWORK_DIMS, float> *map);
	void crackPoints();
	void delaunay();
	void removeBounds();

	std::vector<Instance *> _instances;
	TorsionCluster *_cluster = nullptr;
	ClusterTSNE *_tsne = nullptr;
	Mapping<NETWORK_DIMS, float> *_blueprint = nullptr;
	std::vector<Face<0, NETWORK_DIMS, float> *> _toDelete;

	Entity *_entity = nullptr;
	
	std::map<int, Instance *> _point2Instance;
	std::map<Instance *, int> _instance2Point;
};

#endif
