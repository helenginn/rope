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

#include "Cartographer.h"
#include <vagabond/utils/Mapping.h>
#include <vagabond/core/Network.h>
#include <vagabond/core/CompareDistances.h>
#include <vagabond/core/MappingToMatrix.h>
#include <vagabond/core/SpecificNetwork.h>
#include <vagabond/core/SquareSplitter.h>

Cartographer::Cartographer(Entity *entity, std::vector<Instance *> instances)
{
	_instances = instances;
	_entity = entity;

}

void Cartographer::setup()
{
	makeMapping();

	_mat2Map = new MappingToMatrix(*_mapped);
	_mat2Map->rasterNetwork(_specified);
}

void Cartographer::makeMapping()
{
	Network *net = new Network(_entity, _instances);
	_network = net;
	net->setup();

	_mapped = _network->blueprint();
	_specified = _network->specificForInstance(_instances[0]);
	_specified->setResponder(this);
}

void Cartographer::checkTriangles()
{
	for (size_t i = 0; i < _mapped->faceCount(); i++)
	{
		scoreForTriangle(i);
	}
}

void Cartographer::scoreForTriangle(int idx)
{
	std::vector<std::vector<float>> points = _mat2Map->carts_for_triangle(idx);
	
	if (_distMat.rows > 0)
	{
		zeroMatrix(&_distMat);
	}

	std::vector<int> tickets;
	for (std::vector<float> &p : points)
	{
		int ticket = _specified->submitJob(true, p);
		tickets.push_back(ticket);
	}
	
	_specified->retrieve();
	
	float total = 0; float count = 0;
	for (int &ticket : tickets)
	{
		float sc = _specified->deviation(ticket);
		total += sc; count++;
	}
	
	total /= count;
	std::cout << "Total for " << idx << ": " << total << " over " << 
	count << std::endl;
	
	for (size_t i = 0; i < _distMat.rows * _distMat.cols; i++)
	{
		_distMat.vals[i] /= (float)points.size();
	}

}

void Cartographer::sendObject(std::string tag, void *object)
{
	if (tag == "atom_map")
	{
		AtomPosMap *aps = static_cast<AtomPosMap *>(object);
		CompareDistances cd(*aps);
		PCA::Matrix contrib = cd.matrix();
		
		std::unique_lock<std::mutex> lock(_mDist);
		
		if (_distMat.rows == 0)
		{
			_distMat = contrib;
		}
		else
		{
			for (size_t i = 0; i < _distMat.rows * _distMat.cols; i++)
			{
				_distMat.vals[i] += contrib.vals[i];
			}
		}
	}
}

PCA::Matrix &Cartographer::matrix()
{
	return _mat2Map->matrix();
}
