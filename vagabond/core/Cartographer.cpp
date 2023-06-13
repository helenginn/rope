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

void Cartographer::assessSplits(int idx)
{
	SquareSplitter sqsp(_distMat);
	std::vector<int> splits = sqsp.splits();
	
	_faceBits[idx].problems.clear();
	
	for (size_t i = 0; i < splits.size(); i++)
	{
		if (splits[i] == 0)
		{
			continue;
		}

		Atom *atom = _atoms[splits[i] - 1];
		
		for (size_t j = 0; j < atom->parameterCount(); j++)
		{
			Parameter *p = atom->parameter(j);
			if (p->atomIsCentral(atom) && p->coversMainChain())
			{
				_faceBits[idx].problems.push_back(p);
			}
		}
	}
}

void Cartographer::checkTriangles(bool identify_torsions)
{
	_mapped->update();
	for (size_t i = 0; i < _mapped->faceCount(); i++)
	{
		preparePoints(i);
		scoreForTriangle(i, false, identify_torsions);
		
		if (identify_torsions)
		{
			assessSplits(i);
		}
	}

	_mat2Map->normalise();
	sendResponse("update_map", nullptr);
	
}

void Cartographer::preparePoints(int idx)
{
	std::vector<std::vector<float>> points = _mat2Map->carts_for_triangle(idx);
	_faceBits[idx].points = points;
	
	std::vector<int> all_triangles;
	_mapped->face_indices_for_point(idx, all_triangles);
	
	for (const int &tidx : all_triangles)
	{
		Points points = _mat2Map->carts_for_triangle(tidx);
		Points &wider = _faceBits[idx].wider;
		wider.reserve(wider.size() + points.size());
		wider.insert(wider.end(), points.begin(), points.end());
	}

}

float Cartographer::scoreForTriangle(int idx, bool wide, bool identify_torsions)
{
	float score = 0;
	if (wide)
	{
		score = scoreForPoints(_faceBits[idx].wider, identify_torsions);
	}
	else
	{
		score = scoreForPoints(_faceBits[idx].points, identify_torsions);
	}

	return score;
}

float Cartographer::scoreForPoints(const Cartographer::Points &points, 
                                   bool identify_torsions)
{
	if (_distMat.rows > 0)
	{
		zeroMatrix(&_distMat);
	}

	struct TicketPoint
	{
		int ticket;
		std::vector<float> point;
	};

	std::vector<TicketPoint> tickets;
	for (const std::vector<float> &p : points)
	{
		int ticket = _specified->submitJob(identify_torsions, p);
		tickets.push_back({ticket, p});
		_specified->retrieve();
	}
	
	
	float total = 0; float count = 0;
	for (TicketPoint &tp : tickets)
	{
		float sc = _specified->deviation(tp.ticket);
		_mat2Map->insertScore(sc, tp.point);
		total += sc; count++;
	}
	
	total /= count;
	
	if (identify_torsions)
	{
		for (size_t i = 0; i < _distMat.rows * _distMat.cols; i++)
		{
			_distMat.vals[i] /= (float)points.size();
		}

		sendResponse("atom_matrix", &_distMat);
		_received = 0;
	}

	sendResponse("update_map", nullptr);

	return total;
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
			_atoms = cd.atoms();
		}
		else
		{
			for (size_t i = 0; i < _distMat.rows * _distMat.cols; i++)
			{
				_distMat.vals[i] += contrib.vals[i];
			}
			_received++;
		}
	}
}

PCA::Matrix &Cartographer::matrix()
{
	return _mat2Map->matrix();
}

void Cartographer::run(Cartographer *cg)
{
	cg->checkTriangles();
	cg->flipTriangles();
	
	cg->sendResponse("refined", nullptr);
}

void Cartographer::flipTriangles()
{
	for (int i = 0; i < _mapped->faceCount(); i++)
	{
		flipTriangle(i);
	}
	
	checkTriangles(false);
}

void Cartographer::flipPoint(Mapped<float> *map, int idx, int num_360s)
{
	float add = num_360s * 360.f;
	float t = map->get_value(idx);
	t += add;
	map->alter_value(idx, t);
}

void Cartographer::flipTriangle(int idx)
{
	std::cout << "LOOKING AT TRIANGLE " << idx << std::endl;
	FaceBits &fb = _faceBits[idx];

	for (Parameter *problem : fb.problems)
	{
		Mapped<float> *map = _specified->mapForParameter(problem);
		if (!map)
		{
			continue;
		}

		std::cout << problem->desc() << " " << 
		problem->residueId().str() << std::endl;

		bool changed = flipTriangleTorsion(map, idx);
		
		if (changed)
		{
//			break;
		}
	}
	
	scoreForTriangle(idx, true, false);
}

bool Cartographer::flipTriangleTorsion(Mapped<float> *map, int idx)
{
	std::vector<int> pIndices;
	map->point_indices_for_face(idx, pIndices);

	float orig = map->get_value(pIndices[0]);
	float min = orig; float max = orig;
	float ave = 0;
	for (const int &pidx : pIndices) 
	{
		float next = map->get_value(pidx);
		min = std::min(min, next);
		max = std::max(min, next);
		ave += next;
	}
	
	ave /= (float)pIndices.size();
	
	if (max - min < 30)
	{
		return false;
	}

	float current = scoreForTriangle(idx, true, false);
	std::cout << "TRIANGLE " << idx << " : " << current << std::endl;
	bool changed = false;
	
	for (const int &pidx : pIndices) 
	{
		float orig = map->get_value(pidx);
		std::cout << "Angle: " << orig << ", ";
		
		int n360s = (orig - ave) > 0 ? -1 : 1;
		flipPoint(map, pidx, n360s);
		float update = map->get_value(pidx);
		std::cout << " ->: " << update << ", score ";
		float next = scoreForTriangle(idx, true, false);

		std::cout << current << " vs " << next << std::endl;
		
		if (next < current)
		{
			changed = true;
			current = next;
		}
		else
		{
			flipPoint(map, pidx, n360s * -1);
		}
	}
	
	std::cout << (changed ? "changed, now " : "Now ") << current << std::endl;
	return changed;
}
