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
#include <vagabond/utils/Hypersphere.h>
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

		for (size_t k = 0; k < 2; k++)
		{
			Atom *atom = _atoms[splits[i] - k];

			for (size_t j = 0; j < atom->parameterCount(); j++)
			{
				Parameter *p = atom->parameter(j);
				if (p->hasAtom(atom) && p->coversMainChain())
				{
					_faceBits[idx].problems.push_back(p);
				}
			}
		}
	}
}

void Cartographer::checkTriangles(ScoreMode mode)
{
	_mapped->update();
	for (size_t i = 0; i < _mapped->faceCount(); i++)
	{
		preparePoints(i);
		scoreForTriangle(i, mode);
		
		if (mode == AssessSplits)
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
	std::cout << "face " << idx << " points: " << points.size() << std::endl;
	_faceBits[idx].points = points;
	
	std::vector<int> all_triangles;
	_mapped->face_indices_for_point(idx, all_triangles);
	
	for (const int &tidx : all_triangles)
	{
		Points points = _mat2Map->carts_for_triangle(tidx);
	}

}

float Cartographer::scoreForTriangle(int idx, ScoreMode mode)
{
	float score = scoreForPoints(_faceBits[idx].points, mode);
	return score;
}

template <class Class>
float submitJobs(const Cartographer::Points &points,
                 bool grab_pos, SpecificNetwork *specific,
                 Class *handler, void(Class::*handle)(float, std::vector<float> &))
{
	struct TicketPoint
	{
		int ticket;
		std::vector<float> point;
	};

	std::vector<TicketPoint> tickets;
	for (const std::vector<float> &p : points)
	{
		int ticket = specific->submitJob(grab_pos, p);
		tickets.push_back({ticket, p});
	}
	
	specific->retrieve();
	
	float total = 0; float count = 0;
	for (TicketPoint &tp : tickets)
	{
		float sc = specific->deviation(tp.ticket);
		total += sc; count++;
		
		(handler->*handle)(sc, tp.point);
	}
	
	total /= count;

	return total;
}

void Cartographer::displayDistances()
{
	sendResponse("atom_matrix", &_distMat);

}

void Cartographer::divideDistances(const size_t &num_points)
{
	for (size_t i = 0; i < _distMat.rows * _distMat.cols; i++)
	{
		_distMat.vals[i] /= (float)num_points;
	}
	_received = 0;
}

float Cartographer::distanceScore(float total)
{
	const float nn = _distMat.rows * _distMat.cols;
	float sum = 0;
	for (size_t i = 0; i < nn; i++)
	{
		sum += fabs(_distMat.vals[i]);
	}

	return sum / nn;
}

float Cartographer::scoreForPoints(const Cartographer::Points &points, 
                                   std::function<float(float)> score_func)
{
	return processScores(points, true, score_func);
}

std::function<float(float)> Cartographer::limitedDistanceScore()
{
	std::function<float(float)> func;

	return func;
}

std::function<float(float)> Cartographer::score_func(Cartographer::ScoreMode 
                                                     options)
{
	std::function<float(float)> func = [](float simple_score)
	{
		return simple_score;
	};

	if (options == DistanceScore)
	{
		func = [this](float simple_score)
		{
			return this->distanceScore(simple_score);
		};
	}

	return func;
}

float Cartographer::scoreForPoints(const Cartographer::Points &points, 
                                  Cartographer::ScoreMode options)
{
	std::function<float(float)> func = score_func(options);
	return processScores(points, true, func);
}

float Cartographer::processScores(const Cartographer::Points &points, 
                                  bool grab_positions,
                                  std::function<float(float)> process)
{
	if (_distMat.rows > 0)
	{
		zeroMatrix(&_distMat);
	}

	float total = submitJobs(points, grab_positions, _specified,
	                         _mat2Map, &MappingToMatrix::insertScore);
	
	if (grab_positions)
	{
		this->divideDistances(points.size());
		this->displayDistances();
		total = process(total);
	}


	sendResponse("update_map", nullptr);

	return total;
}

void Cartographer::sendObject(std::string tag, void *object)
{
	if (tag == "atom_map")
	{
		AtomPosMap *aps = static_cast<AtomPosMap *>(object);
		CompareDistances cd;
		cd.process(*aps);
		PCA::Matrix contrib = cd.matrix();
		
		std::unique_lock<std::mutex> lock(_mDist);
		
		if (_distMat.rows == 0)
		{
			_distMat = contrib;
			_atoms = cd.leftAtoms();
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
	cg->checkTriangles(AssessSplits);
	cg->flipPoints();
	cg->checkTriangles(DisplayMatrix);
	
	cg->sendResponse("refined", nullptr);
}

Mappable<float> *Cartographer::extendFace(std::vector<int> &pidxs, int &tidx)
{
	for (size_t i = 0; i < _mapped->pointCount(); i++)
	{
		if (std::find(pidxs.begin(), pidxs.end(), i) != pidxs.end())
		{
			continue; // got it already
		}
		
		std::vector<int> tIndices;
		_mapped->face_indices_for_point(i, tIndices);
		
		for (const int &t : tIndices)
		{
			std::vector<int> pIndices;
			_mapped->point_indices_for_face(t, pIndices);
			
			bool found_all = true;
			for (const int &p : pIndices)
			{
				if (p != i && 
				    std::find(pidxs.begin(), pidxs.end(), p) == pidxs.end())
				{
					found_all = false;
					break;
				}
			}
			
			if (found_all)
			{
				tidx = t;
				pidxs.push_back(i);
				return _mapped->simplex_for_points(pIndices);
			}
		}
	}
	
	return nullptr;
}

Mappable<float> *Cartographer::bootstrapFace(std::vector<int> &pidxs)
{
	for (size_t i = 0; i < _mapped->pointCount(); i++)
	{
		if (std::find(pidxs.begin(), pidxs.end(), i) != pidxs.end())
		{
			continue; // got it already
		}
		
		std::vector<int> copy = pidxs;
		copy.push_back(i);
		
		Mappable<float> *map = _mapped->simplex_for_points(copy);
		if (!map) continue;

		pidxs = copy;
		return map;
	}
	
	return nullptr;
}

int Cartographer::bestStartingPoint()
{
	int best = -1;
	int score = INT_MAX;
	for (size_t i = 0; i < _mapped->pointCount(); i++)
	{
		std::vector<Mappable<float> *> simplices;
		simplices = _mapped->simplicesForPointDim(i, 1);
		if ((int)simplices.size() >= score)
		{
			continue;
		}
		std::vector<int> tridxs;
		_mapped->face_indices_for_point(i, tridxs);
		if (tridxs.size() > 0)
		{
			score = simplices.size();
			best = i;
		}
	}

	return best;
}

void Cartographer::flipPoints()
{
	_mapped->redo_bins();
	int best_start = bestStartingPoint();
	std::cout << "best start: " << best_start << std::endl;

	std::vector<int> todo(1, best_start);
	
	Mappable<float> *face = bootstrapFace(todo);
	flipPointsFor(face, todo);
	
	while (face->n() < _mapped->n())
	{
		face = bootstrapFace(todo);
		
		if (!face)
		{
			break;
		}

		flipPointsFor(face, todo);
	}

	checkTriangles(DisplayMatrix);

	int t = -1;
	while (true)
	{
		face = extendFace(todo, t);
		if (face == nullptr)
		{
			break;
		}

		flipPointsFor(face, todo);
		scoreForTriangle(t, DisplayMatrix);
	}
}

Parameter *get_param(Cartographer::ParamMap pair)
{
	return pair.first;
};

Mapped<float> *get_map(Cartographer::ParamMap pair)
{
	return pair.second;
};

template <typename T, typename P, typename Q>
std::vector<T> extract(std::vector<std::pair<P, Q>> pairs, 
                       T(*extract_method)(std::pair<P, Q>))
{
	std::vector<T> bits_only; bits_only.reserve(pairs.size());
	
	for (std::pair<P, Q> pair : pairs)
	{
		bits_only.push_back(extract_method(pair));
	}

	return bits_only;
}

Cartographer::ParamMapGroups 
Cartographer::splitParameters(const ParamMapGroup &list)
{
	ParamMapGroups ret;
	ParamMapGroup current;
	
	for (const Cartographer::ParamMap &pair : list)
	{
		Parameter *param = pair.first;

		if (current.size() == 0)
		{
			current.push_back(pair);
		}
		else if (param->residueId().as_num() -
		         current.back().first->residueId().as_num() < 5)
		{
			current.push_back(pair);
		}
		else
		{
			ret.push_back(current);
			current.clear();
		}
	}

	ret.push_back(current);
	return ret;
}

void Cartographer::flipPointsFor(Mappable<float> *face,
                                 const std::vector<int> &pIndices)
{
	int pidx = pIndices.back();
	Points test_points = cartesiansForFace(face);
	std::vector<ParamMap> param_maps;
	param_maps = torsionMapsFor(pidx);

	ParamMapGroups groups = splitParameters(param_maps);
	std::vector<Parameter *> params = extract(param_maps, &get_param);
	std::cout << "Parameter number: " << params.size() << std::endl;
	std::cout << "Parameter groups: " << groups.size() << std::endl;

	std::vector<Mapped<float> *> maps = extract(param_maps, &get_map);
	permute(maps, test_points, pidx);
}

Cartographer::Points Cartographer::cartesiansForFace(Mappable<float> *face)
{
	Points carts;
	const int dims = face->n();
	
	if (dims == 1)
	{
		float steps = 10;
		for (size_t i = 0; i < steps; i++)
		{
			float frac = (float)i / float(steps);
			std::vector<float> bc = {frac, 1 - frac};
			std::vector<float> cart = face->barycentric_to_point(bc);
			
			carts.push_back(cart);
		}
	}
	else if (dims > 1)
	{
		int num = 2 * (dims + 2) * (dims + 2);
		Hypersphere sph(dims, num);
		sph.prepareFibonacci();
		
		for (size_t i = 0; i < sph.count(); i++)
		{
			std::vector<float> bc = sph.point(i);
			
			float sum = 0;
			for (float &f : bc)
			{
				f = fabs(f);
				sum += f;
			}
			for (float &f : bc)
			{
				f /= sum;
			}

			std::vector<float> cart = face->barycentric_to_point(bc);
			carts.push_back(cart);
		}
	}
	
	return carts;
}

void Cartographer::getPoints(std::vector<Mapped<float> *> &maps,
                             std::vector<float> &values, int pidx)
{
	values.clear();
	for (size_t i = 0; i < maps.size(); i++)
	{
		const float &new_val = maps[i]->get_value(pidx);
		values.push_back(new_val);
	}
}

void Cartographer::setPoints(std::vector<Mapped<float> *> &maps,
                             const std::vector<float> &values, int pidx)
{
	for (size_t i = 0; i < maps.size(); i++)
	{
		const float &new_val = values[i];
		maps[i]->alter_value(pidx, new_val);
	}
}

bool check_indices(int tridx, Mapped<float> *const &map, 
                   float &ave)
{
	std::vector<int> pIndices;
	map->point_indices_for_face(tridx, pIndices);

	float min = FLT_MAX; float max = -FLT_MAX;

	ave = 0;
	for (const int &pidx : pIndices) 
	{
		float next = map->get_value(pidx);
		min = std::min(min, next);
		max = std::max(max, next);
		ave += next;
	}
	
	ave /= (float)pIndices.size();

	return (max - min) > 30;
}

std::vector<Cartographer::ParamMap> Cartographer::torsionMapsFor(int pidx)
{
	std::vector<int> tridxs;
	_mapped->face_indices_for_point(pidx, tridxs);
	std::vector<ParamMap> maps; 

	for (int &idx : tridxs)
	{
		FaceBits &fb = _faceBits[idx];
		for (Parameter *problem : fb.problems)
		{
			Mapped<float> *map = _specified->mapForParameter(problem);
			if (!map)
			{
				continue;
			}
			
			float average = 0;
			bool ok = check_indices(idx, map, average);
			
			if (!ok) { continue; }
			
			auto comp = [map](const ParamMap &pair) -> bool
			{
				return (pair.second == map);
			};

			if (std::find_if(maps.begin(), maps.end(), comp) == maps.end())
			{
				maps.push_back(std::make_pair(problem, map));
			}
		}
	}
	
	auto sort_by_res = [](const ParamMap &a, const ParamMap &b)
	{
		if (a.first->residueId().as_num() == b.first->residueId().as_num())
		{
			return a.first->desc() < b.first->desc();
		}

		return a.first->residueId().as_num() < b.first->residueId().as_num();
	};
	
	std::sort(maps.begin(), maps.end(), sort_by_res);
	return maps;
}

void Cartographer::permute(std::vector<Mapped<float> *> &maps, 
                           std::function<float()> score, int pidx)
{
	
	std::vector<std::vector<int>> perms;
	int perm_length = std::min(5, (int)maps.size());
	perms = permutations(perm_length, 3);

	std::vector<float> old_vals; 
	getPoints(maps, old_vals, pidx);
	float current = score();
	std::cout << "starting: " << current << std::endl;
	std::vector<float> best = old_vals;
	
	int start = 0;
	while (true)
	{
		int end = start + perm_length;

		if (end > maps.size())
		{
			break;
		}

		for (const std::vector<int> &perm : perms)
		{
			std::vector<float> new_vals = best;

			int n = 0;
			auto lmb = [](const int &p) -> float
			{
				if (p == 0) return 0;
				if (p == 1) return -360;
				else return 360;
			};

			for (size_t i = start; i < end; i++)
			{
				const float diff = old_vals[i];

				float flipped = lmb(perm[n]) + old_vals[i];
				new_vals[i] = flipped;

				n++;
			}
			
			setPoints(maps, new_vals, pidx);

			float next = score();

			if (current > next)
			{
				std::cout << "IMPROVEMENT" << std::endl;
				std::cout << "next: " << next << std::endl;
				current = next;
				best = new_vals;
			}
		}
		start++;
	}

	setPoints(maps, best, pidx);

}

void Cartographer::permute(std::vector<Mapped<float> *> &maps,
                           Points all_points, int pidx)
{
	std::cout << "Number of parameters: " << maps.size() << std::endl;
	std::cout << "Number of points to test: " << all_points.size() << std::endl;

	std::function<float()> score = [this, all_points]()
	{
		return this->scoreForPoints(all_points, Simple);
	};
	
	permute(maps, score, pidx);
}
