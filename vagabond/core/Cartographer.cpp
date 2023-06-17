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

#include <vagabond/utils/Mapping.h>
#include <vagabond/utils/Hypersphere.h>
#include "Network.h"
#include "ProblemPrep.h"
#include "MappingToMatrix.h"
#include "SpecificNetwork.h"
#include "Cartographer.h"

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
	_prepwork = new ProblemPrep(_specified, _mapped);
}

void Cartographer::checkTriangles(ScoreMap::Mode mode)
{
	_mapped->update();
	for (size_t i = 0; i < _mapped->faceCount(); i++)
	{
		preparePoints(i);
		scoreForPoints(_pointsInTriangles[i].points, mode);
		
		if (mode == ScoreMap::AssessSplits)
		{
			_prepwork->processMatrixForTriangle(i, _distMat, _atoms);
		}
	}

	_mat2Map->normalise();
	sendResponse("update_map", nullptr);
	
}

void Cartographer::preparePoints(int idx)
{
	std::vector<std::vector<float>> points = _mat2Map->carts_for_triangle(idx);
	_pointsInTriangles[idx].points = points;
}

float Cartographer::scoreForTriangle(int idx, ScoreMap::Mode mode)
{
	float score = scoreForPoints(_pointsInTriangles[idx].points, mode);
	return score;
}

ScoreMap Cartographer::basicScorer(ScoreMap::Mode options)
{
	ScoreMap scorer(_mapped, _specified);
	
	MappingToMatrix *m2m = _mat2Map;
	auto handle = [m2m](float s, std::vector<float> &ps)
	{
		m2m->insertScore(s, ps);
	};

	scorer.setIndividualHandler(handle);
	scorer.setMode(options);

	return scorer;
}

float Cartographer::scoreForPoints(const Points &points, ScoreMap::Mode options)
{
	ScoreMap scorer = basicScorer(options);
	return scoreWithScorer(points, scorer);
}

float Cartographer::scoreWithScorer(const Points &points, ScoreMap scorer)
{
	float result = scorer.scoreForPoints(points);

	sendResponse("update_map", nullptr);

	if (scorer.hasMatrix())
	{
		_distMat = scorer.matrix();
		_atoms = scorer.atoms();
		sendResponse("atom_matrix", &_distMat);
	}

	return result;

}

PCA::Matrix &Cartographer::matrix()
{
	return _mat2Map->matrix();
}

void Cartographer::assess(Cartographer *cg)
{
	cg->checkTriangles(ScoreMap::AssessSplits);
	cg->sendResponse("done", nullptr);
}

void Cartographer::flip(Cartographer *cg)
{
	try
	{
		cg->flipPoints();
		cg->checkTriangles(ScoreMap::Distance);
		cg->sendResponse("done", nullptr);
	}
	catch (const int &ret)
	{
		// stopping
		assess(cg);
		cg->_stop = false;
		cg->sendResponse("paused", nullptr);
	}

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

int Cartographer::bestStartingPoint(std::vector<int> &ruled_out)
{
	int best = -1;
	int score = INT_MAX;
	std::vector<int> random_trials;

	for (size_t i = 0; i < _mapped->pointCount(); i++)
	{
		random_trials.push_back(i);
	}
	
	std::random_shuffle(random_trials.begin(), random_trials.end());

	for (size_t i = 0; i < _mapped->pointCount(); i++)
	{
		int idx = random_trials[i];

		if (std::find(ruled_out.begin(), ruled_out.end(), idx) != ruled_out.end())
		{
			continue;
		}

		std::vector<Mappable<float> *> simplices;
		simplices = _mapped->simplicesForPointDim(idx, 1);
		if ((int)simplices.size() >= score)
		{
			continue;
		}
		std::vector<int> tridxs;
		_mapped->face_indices_for_point(idx, tridxs);
		if (tridxs.size() > 0)
		{
			score = simplices.size();
			best = idx;
		}
	}

	return best;
}

void Cartographer::flipPoints()
{
	_mapped->redo_bins();
	std::vector<int> bad;
	std::vector<int> todo;
	
	Mappable<float> *face = nullptr;
	while (true)
	{
		int best_start = bestStartingPoint(bad);
		todo = std::vector<int>(1, best_start);

		face = bootstrapFace(todo);
		if (face)
		{
			break;
		}

		// got a bad vertex
		bad.push_back(best_start);
	}

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

	int t = -1;
	while (true)
	{
		face = extendFace(todo, t);
		if (face == nullptr)
		{
			break;
		}

		flipPointsFor(face, todo);
		scoreForTriangle(t, ScoreMap::Distance);
	}
}

void Cartographer::flipGroup(Mappable<float> *face, int g, int pidx)
{
	std::vector<Parameter *> params = _prepwork->paramsFor(pidx, g);
	Points points = cartesiansForFace(face, params.size());
	
	std::function<bool(Atom *const &atom)> left, right;
	_prepwork->setFilters(pidx, g, left, right);

	ScoreMap scorer = basicScorer(ScoreMap::Distance);
	scorer.setFilters(left, right);

	std::function<float()> score = [this, scorer, points]()
	{
		return this->scoreWithScorer(points, scorer);
	};

	std::cout << "Number of parameters to permute: " << params.size() << std::endl;
	std::cout << "Number of points to test: " << points.size() << std::endl;

	float start = score();
	std::cout << "Start: " << start << std::endl;
	if (start < 0.1)
	{
		return;
	}
	
	permute(params, score, pidx);
}

void Cartographer::flipPointsFor(Mappable<float> *face,
                                 const std::vector<int> &pIndices)
{
	int pidx = pIndices.back();
	
	size_t grpCount = _prepwork->groupCount(pidx);
	std::cout << "Number of groups: " << grpCount << std::endl;
	
	for (size_t g = 0; g < grpCount; g++)
	{
		flipGroup(face, g, pidx);
	}

}

Cartographer::Points Cartographer::cartesiansForFace(Mappable<float> *face,
                                                     int paramCount)
{
	Points carts;
	const int dims = face->n();
	
	if (dims == 1)
	{
		float steps = paramCount * 2;
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
		int num = paramCount * (dims + 2) * 2;
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

std::vector<Mapped<float> *> parametersToMaps(SpecificNetwork *sn, 
                                              std::vector<Parameter *> &params)
{
	std::vector<Mapped<float> *> maps;

	for (Parameter *p : params)
	{
		maps.push_back(sn->mapForParameter(p));
	}

	return maps;
}

void Cartographer::permute(std::vector<Parameter *> &params, 
                           std::function<float()> score, int pidx)
{
	std::vector<Mapped<float> *> maps = parametersToMaps(_specified, params);

	std::vector<std::vector<int>> perms;
	int perm_length = std::min(5, (int)maps.size());
	perms = permutations(perm_length, 3);
	std::random_shuffle(perms.begin(), perms.end());

	std::vector<float> old_vals; 
	getPoints(maps, old_vals, pidx);
	float current = score();
	sendResponse("update_score", &current);
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
				float flipped = lmb(perm[n]) + old_vals[i];
				new_vals[i] = flipped;

				n++;
			}
			
			setPoints(maps, new_vals, pidx);

			float next = score();

			if (current > next)
			{
				std::cout << "IMPROVEMENT " << next << std::endl;
				current = next;
				sendResponse("update_score", &current);
				best = new_vals;
			}
			
			if (_skip)
			{
				setPoints(maps, best, pidx);
				_skip = false;
				return;
			}
			
			if (_stop)
			{
				setPoints(maps, best, pidx);
				throw 0;
			}
		}
		start++;
	}

	setPoints(maps, best, pidx);
}

