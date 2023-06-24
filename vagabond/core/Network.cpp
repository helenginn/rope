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

#include "Network.h"
#include "Entity.h"
#include "MetadataGroup.h"
#include "RopeCluster.h"
#include "SpecificNetwork.h"
#include "ModelManager.h"
#include <vagabond/c4x/ClusterTSNE.h>
#include <vagabond/utils/Mapping.h>

Network::Network(Entity *entity, std::vector<Instance *> &instances)
{
	_instances = instances;
	_entity = entity;
}

void Network::makeCluster()
{
	if (_cluster != nullptr)
	{
		delete _cluster;
		_cluster = nullptr;
	}

	MetadataGroup angles = _entity->makeTorsionDataGroup();
	angles.setWhiteList(_instances);
	_cluster = new TorsionCluster(angles);
	_cluster->cluster();
}

void Network::convertToTSNE()
{
	if (_tsne != nullptr)
	{
		delete _tsne;
		_tsne = nullptr;
	}
	
	PCA::Matrix distances = _cluster->dataGroup()->distanceMatrix();
	const PCA::Matrix *start = &_cluster->results();

	_tsne = new ClusterTSNE(distances, start, NETWORK_DIMS);
	_tsne->cluster();
}

void Network::preparePoints()
{
	for (Instance *instance : _instances)
	{
		int idx = _cluster->dataGroup()->indexOfObject(instance);
		std::vector<float> point = _tsne->point(idx);
		point.resize(NETWORK_DIMS);
		int last = _blueprint->add_point(point);
		_point2Instance[last] = instance;
		_instance2Point[instance] = last;
	}
}

void Network::addBounds(Mapping<NETWORK_DIMS, float> *map)
{
	std::vector<float> min, max;
	map->bounds(min, max);
	
	for (int i = 0; i < min.size(); i++)
	{
		min[i] *= 2;
		max[i] *= 2;
	}
	
	const int D = NETWORK_DIMS;

	SharedFace<0, D, float> *mm, *mM, *Mm, *MM;
	mm = new SharedFace<0, D, float>({min[0], min[1]}, -1.0);
	mM = new SharedFace<0, D, float>({min[0], max[1]}, -1.0);
	Mm = new SharedFace<0, D, float>({max[0], min[1]}, -1.0);
	MM = new SharedFace<0, D, float>({max[0], max[1]}, -1.0);
	
	map->add_simplex({mm, mM, Mm});
	map->add_simplex({mM, Mm, MM});
}

void Network::removeBounds()
{
	for (size_t i = 0; i < 4; i++)
	{
		_blueprint->remove_point(_blueprint->pointCount() - 1);
	}
}

void Network::makeBlueprintMapping()
{
	Mapping<NETWORK_DIMS, float> *map = new Mapping<NETWORK_DIMS, float>();
	_blueprint = map;
	preparePoints();
	addBounds(map);
	crackPoints();
	removeBounds();
}

void Network::setup()
{
	if (!_blueprint)
	{
		makeCluster();
		convertToTSNE();
		makeBlueprintMapping();
	}

	_blueprint->update();
}

void Network::crackPoints()
{
	float val = -1;
	for (auto it = _point2Instance.begin(); it != _point2Instance.end(); it++)
	{
		int idx = it->first;
		_blueprint->crack_existing_face(idx);
		_blueprint->alter_value(idx, val);
		val += 3.f / (float)_point2Instance.size();
		delaunay();
	}
}

void Network::delaunay()
{
	_blueprint->delaunay_refine();
}

Mapped<float> *Network::blueprint_scalar_copy()
{
	Mapping<NETWORK_DIMS, float> *copy = new Mapping<NETWORK_DIMS, float>(*_blueprint);
	return copy;
}

Mapped<glm::vec3> *Network::blueprint_vec3_copy()
{
	Mapping<NETWORK_DIMS, glm::vec3> *copy = new Mapping<NETWORK_DIMS, glm::vec3>(*_blueprint);
	return copy;
}

Mapped<Vec3s> *Network::blueprint_vec3s_copy()
{
	Mapping<NETWORK_DIMS, Vec3s> *copy = new Mapping<NETWORK_DIMS, Vec3s>(*_blueprint);
	return copy;
}

SpecificNetwork *Network::specificForInstance(Instance *instance)
{
	SpecificNetwork *sn = new SpecificNetwork(this, instance);
	return sn;
}

std::vector<float> Network::pointForInstance(Instance *instance)
{
	int idx = indexForInstance(instance);
	std::vector<float> cart = _blueprint->point_vector(idx);
	return cart;
}

bool Network::valid_position(const std::vector<float> &vals)
{
	return _blueprint->acceptable_coordinate(vals);

}

void Network::add_info(nlohmann::json &json)
{
	json["entity"] = _entity->name();
	json["blueprint"] = *_blueprint;

	for (Instance *inst : _instances)
	{
		json["instances"].push_back(inst->id());

		int idx = _instance2Point[inst];
		json["indices"][inst->id()] = idx;
	}
}

void Network::get_info(nlohmann::json &json)
{
	_blueprint = new Mapping<NETWORK_DIMS, float>(json["blueprint"]);

	for (std::string id : json["instances"])
	{
		Instance *instance = ModelManager::manager()->instance(id);
		int idx = json["indices"][id];
		
		_point2Instance[idx] = instance;
		_instance2Point[instance] = idx;
	}
}
