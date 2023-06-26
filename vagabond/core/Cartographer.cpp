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
#include "SpecificNetwork.h"
#include "Permuter.h"
#include "Nudger.h"
#include "ProblemPrep.h"
#include "MappingToMatrix.h"
#include "Cartographer.h"

Cartographer::Cartographer(Entity *entity, std::vector<Instance *> instances)
{
	_instances = instances;
	_entity = entity;

}

void Cartographer::supplyExisting(SpecificNetwork *spec)
{
	_specified = spec;
	_network = _specified->network();
	_instances = _network->instances();
	_entity = _network->entity();
}

void Cartographer::setup()
{
	if (!_network || !_specified)
	{
		makeMapping();
	}
	else
	{
		hookReferences();
		_specified->setup();
	}
}

void Cartographer::hookReferences()
{
	_network->setup();
	_mapped = _network->blueprint();
	_prepwork = new ProblemPrep(_specified, _mapped);
	_mat2Map = new MappingToMatrix(*_mapped);
}

void Cartographer::makeMapping()
{
	Network *net = new Network(_entity, _instances);
	_network = net;
	_network->setup();

	_specified = _network->specificForInstance(_instances[0]);
	hookReferences();
}

void Cartographer::checkTriangles(ScoreMap::Mode mode)
{
	for (size_t i = 0; i < _mapped->faceCount(); i++)
	{
		preparePoints(i);
		float result = scoreForPoints(_pointsInTriangles[i].points, mode);
		_pointsInTriangles[i].score = result;
		
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

void Cartographer::sendObject(std::string tag, void *object)
{
	sendResponse(tag, object);
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

void Cartographer::flipPoint(int i, int j)
{
	std::vector<int> todo{i, j};
	std::cout << "Trying to flip torsions " << i << " to " << j << std::endl;
	_mapped->redo_bins();
	Mappable<float> *face = _mapped->simplex_for_points(todo);
	if (!face)
	{
		std::cout << "Didn't find it, are these even connected?" << std::endl;
		return;
	}

	flipPointsFor(face, todo);

}

void Cartographer::flipIdx(Cartographer *cg, int i, int j)
{
	try
	{
		cg->flipPoint(i, j);
		cg->checkTriangles(ScoreMap::Basic);
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

void Cartographer::nudge(Cartographer *cg, const std::vector<float> &point)
{
	try
	{
		std::cout << "Nudging..." << std::endl;
		cg->nudgePoints(point);
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
		
		if (pidxs.size() == 2)
		{
			int tmp = pidxs[0];
			pidxs[0] = pidxs[1];
			pidxs[1] = tmp;
		}

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
	std::vector<Parameter *> params = _prepwork->paramsForPoint(pidx, g);
	Points points = cartesiansForFace(face, 10);
	
	std::function<bool(Atom *const &atom)> left, right;
	_prepwork->setFilters(pidx, g, left, right);

	ScoreMap scorer = basicScorer(ScoreMap::Distance);
	scorer.setFilters(left, right);

	std::function<float()> score = [this, scorer, points]()
	{
		return this->scoreWithScorer(points, scorer);
	};

	std::cout << "Number of parameters to permute: " << params.size() << std::endl;

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
	
	size_t grpCount = _prepwork->groupCountForPoint(pidx);
	std::cout << "Number of groups: " << grpCount << std::endl;
	
	for (size_t g = 0; g < grpCount; g++)
	{
		flipGroup(face, g, pidx);
	}

}

Cartographer::Points Cartographer::cartesiansForPoint(int pidx, int paramCount)
{
	std::vector<int> tIndices;
	_mapped->face_indices_for_point(pidx, tIndices);

	Points carts;

	for (const int &t : tIndices)
	{
		Mappable<float> *face = _mapped->face_for_index(t);
		Points extra = cartesiansForFace(face, paramCount);
		carts.reserve(carts.size() + extra.size());
		carts.insert(carts.end(), extra.begin(), extra.end());
	}
	
	return carts;
}

void Cartographer::cartesiansForTriangle(int tidx, int paramCount,
                                         Cartographer::Points &carts)
{
	_mapped->redo_bins();

	std::vector<int> pIndices;
	_mapped->point_indices_for_face(tidx, pIndices);

	for (const int &p : pIndices)
	{
		Points extra = cartesiansForPoint(p, paramCount);
		carts.reserve(carts.size() + extra.size());
		carts.insert(carts.end(), extra.begin(), extra.end());
	}
	
	for (size_t i = 0; i < pIndices.size(); i++)
	{
		std::vector<int> copy = pIndices;
		copy.erase(copy.begin() + i);
		Mappable<float> *line = _mapped->simplex_for_points(copy);
		
		if (line)
		{
			Points extra = cartesiansForFace(line, paramCount);
			carts.reserve(carts.size() + extra.size());
			carts.insert(carts.end(), extra.begin(), extra.end());
		}
		else
		{
			std::cout << "could not find the line" << std::endl;
		}
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

void Cartographer::nudger(std::vector<Parameter *> &params, 
                           std::function<float()> score, int pidx)
{
	Nudger nudge(_specified, _mapped, _stop, _skip);
	nudge.setResponder(this);
	nudge.bindPointGradients(pidx, params);
	
	std::vector<float> flexes;
	for (size_t i = 0; i < params.size(); i++)
	{
		float flex = _prepwork->flexForParameter(params[i]);
		
		for (size_t j = 0; j < _mapped->n(); j++)
		{
			flexes.push_back(flex * 2);
		}
	}

	nudge.nudge(flexes, score);
}

void Cartographer::permute(std::vector<Parameter *> &params, 
                           std::function<float()> score, int pidx)
{
	Permuter permuter(_specified, _mapped, _stop, _skip);
	permuter.setResponder(this);
	permuter.bindPointValues(pidx, params);
	
	std::vector<std::function<float(int &)>> flips;
	std::vector<Mapped<float> *> maps;
	for (size_t i = 0; i < params.size(); i++)
	{
		if (!_specified->mapForParameter(params[i]))
		{
			continue;
		}
		
		flips.push_back(_prepwork->flipFunction(params[i], pidx));
	}

	permuter.bindFlips(flips);
	permuter.permute(score);
}

void add_triangle_indices(int pidx, std::set<int> &tidxs, Mapped<float> *map)
{
	std::vector<int> tIndices;
	map->face_indices_for_point(pidx, tIndices);

	for (const int &t : tIndices)
	{
		tidxs.insert(t);
	}
}

std::function<float()> Cartographer::scorerForNudge(int tidx, size_t paramCount)
{
	Points points;
	cartesiansForTriangle(tidx, paramCount, points);

	ScoreMap scorer = basicScorer(ScoreMap::Distance);
	std::function<float()> score = [this, scorer, points]()
	{
		return this->scoreWithScorer(points, scorer);
	};
	
	return score;

}

void Cartographer::nudgePoints(const std::vector<float> &point)
{
	int tidx = _mapped->face_idx_for_point(point);
	if (tidx < 0)
	{
		std::cout << "give up, no triangle selected" << std::endl;
		return;
	}

	int grps = _prepwork->groupCountForTriangle(tidx);
	std::cout << "group count: " << grps << std::endl;
	
	for (int i = 0; i < grps; i++)
	{
		std::vector<Parameter *> params = _prepwork->paramsForTriangle(tidx, i);
		_prepwork->filterParameters(params);
		std::cout << "Filtered params: " << params.size() << std::endl;
		
		std::function<float()> score = scorerForNudge(tidx, params.size());
		std::cout << "begin: " << score() << std::endl;

		std::cout << "Parameters for triangle: " << std::endl;
		for (Parameter *p : params)
		{
			std::cout << p->residueId() << ":" << p->desc() << ", ";
		}
		std::cout << std::endl;

		std::vector<int> pIndices;
		_mapped->point_indices_for_face(tidx, pIndices);

		for (int pidx : pIndices)
		{
			nudger(params, score, pidx);
		}
		
		if (_skip)
		{
			_skip = false;
		}
		
		scoreForTriangle(tidx, ScoreMap::Distance);
	}
}
