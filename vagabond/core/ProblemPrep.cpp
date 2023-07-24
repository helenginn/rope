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
#include <vagabond/utils/maths.h>
#include "ProblemPrep.h"
#include "SpecificNetwork.h"
#include "SquareSplitter.h"
#include "Atom.h"

float check_indices(int tridx, Mapped<Floats> *const &map, int idx,
                    float *average = nullptr)
{
	if (!map) { return 0; }
	std::vector<int> pIndices;
	map->point_indices_for_face(tridx, pIndices);

	float min = FLT_MAX; float max = -FLT_MAX;

	float ave = 0;
	for (const int &pidx : pIndices) 
	{
		float next = map->get_value(pidx)[idx];
		min = std::min(min, next);
		max = std::max(max, next);
		ave += next;
	}
	
	ave /= (float)pIndices.size();
	
	if (average)
	{
		*average = ave;
	}

	return (max - min);
}

ProblemPrep::ProblemPrep(SpecificNetwork *sn, Mapped<float> *mapped)
{
	_specified = sn;
	_mapped = mapped;

	auto flex = [sn](Parameter *problem, int tridx) -> float
	{
		auto pair = sn->bondMappingFor(problem);
		Mapped<Floats> *map = pair.first;
		int idx = pair.second;
		float ave = check_indices(tridx, map, idx);
		return ave;
	};
	_flex = flex;

	auto variation = [flex](Parameter *problem, int tridx) -> bool
	{
		float ave = flex(problem, tridx);
		bool ret = (ave > 30);
		return ret;
	};

	std::function<bool(const Parameter *)> ramaFilter;
	ramaFilter = [sn](const Parameter *problem) -> bool
	{
		if (sn->bondMappingFor(problem).second < 0)
		{
			return false;
		}
		return problem->coversMainChain() && !problem->isPeptideBond();
	};
	_ramaFilter = ramaFilter;

	_filter = [sn, ramaFilter](Parameter *problem, int tridx) -> bool
	{
		bool answer = (ramaFilter(problem));
		return answer;
	};
}

void ProblemPrep::filterParameters(Parameters &params)
{
	Parameters redo;
	
	for (Parameter *param : params)
	{
		if (_ramaFilter(param))
		{
			redo.push_back(param);
		}
	}

	params = redo;
}

std::function<float(int &)> ProblemPrep::flipFunction(Parameter *param, int pidx)
{
	auto pair = _specified->bondMappingFor(param);
	Mapped<Floats> *map = pair.first;
	int idx = pair.second;

	std::vector<int> tIndices;
	_mapped->face_indices_for_point(pidx, tIndices);

	float mine = map->get_value(pidx)[idx];
	float ave = 0;
	float sq = 0;
	float count = 0;

	for (const int &tidx : tIndices)
	{
		std::vector<int> pIndices;
		map->point_indices_for_face(tidx, pIndices);

		for (const int &pidx : pIndices) 
		{
			float next = map->get_value(pidx)[idx];
			ave += next;
			sq += next * next;
			count++;
		}
	}
	ave /= (float)count;
	float stdev = sqrt(sq / count - ave * ave);

	std::function<float(int &)> direction = [mine, ave](int &idx) -> float
	{
		float ret = mine;
		matchDegree(ave, ret);
		if (idx == 0)
		{
			return ret;
		}
		else if (idx == 1)
		{
			return ret < ave ? ret + 360 : ret - 360;
		}
		else
		{
			return ret > ave ? ret + 360 : ret - 360;
		}
	};
	
	return direction;
}

const Atom *atomFrom( const std::vector<Atom *> &atoms, int idx)
{
	if (idx < 0 || idx >= atoms.size())
	{
		return nullptr;
	}
	
	return atoms.at(idx);
}

void ProblemPrep::addAtomToParams(ProblemPrep::ParamSet &params, const Atom *atom, 
                                  float drop)
{
	if (atom == nullptr)
	{
		return;
	}

	for (size_t j = 0; j < atom->parameterCount(); j++)
	{
		Parameter *p = atom->parameter(j);
		params.insert(p);
		_splitDrop[p] = drop;
	}
}

size_t ProblemPrep::groupCountForTriangle(int tidx)
{
	return _problemsForTriangles[tidx].grouped.size();
}

size_t ProblemPrep::groupCountForPoint(int pidx)
{
	return _problemsForPoints[pidx].grouped.size();
}

ProblemPrep::Parameters ProblemPrep::paramsForPoint(int pidx, int grp)
{
	return _problemsForPoints[pidx].grouped[grp];
}

ProblemPrep::Parameters ProblemPrep::paramsForTriangle(int tidx, int grp)
{
	return _problemsForTriangles[tidx].grouped[grp];
}


ProblemPrep::AtomFilter chain_and(ProblemPrep::AtomFilter first, 
                                  ProblemPrep::AtomFilter second)
{
	if (!second)
	{
		return first;
	}

	return [first, second](Atom *const &atom)
	{
		return first(atom) && second(atom);
	};
}

ProblemPrep::AtomFilter residue_boundary(bool less, Parameter *param)
{
	int bound = 0;
	if (!param)
	{
		bound = less ? INT_MAX : -INT_MAX;
	}
	else
	{
		bound = param->residueId().as_num();
	}

	ProblemPrep::AtomFilter filter = [less, bound](Atom *const &atom)
	{
		if (less)
		{
			return atom->residueId().as_num() < bound;
		}
		else
		{
			return atom->residueId().as_num() > bound;
		}
	};
	
	std::cout << "Applying filter: " << (less ? "less than " : "more than ")
	<< bound << std::endl;
	
	return filter;
}

void ProblemPrep::setFilters(int pidx, int grp,
                             ProblemPrep::AtomFilter &left,
                             ProblemPrep::AtomFilter &right, bool ends)
{
	ProblemPrep::AtomFilter all = [](Atom *const &atom) { return true; };
	left = all; right = all;

	if (_problemsForPoints.size() <= pidx)
	{
		throw std::runtime_error("Problem: beyong end of point problem array");
	}

	Problems &probs = _problemsForPoints[pidx];
	
	std::cout << "Starting group: " << grp << " for point " << pidx << std::endl;
	for (size_t i = 0; i < probs.grouped[grp].size(); i++)
	{
		Parameter *&p = probs.grouped[grp][i];
		std::cout << p->residueId() << " ";
		std::cout << p->desc() << " ";
		std::cout << _flexResults[p] << std::endl;
	}
	
	if (probs.grouped[grp].size() == 0)
	{
		return;
	}

	{
		std::cout << "Left: " << std::endl;
		Parameter *start = probs.less_than_lower_bound(grp);
		if (!ends) start = nullptr;
		Parameter *end = probs.more_than_lower_bound(grp);
		left = chain_and(left, residue_boundary(false, start));
		left = chain_and(left, residue_boundary(true, end));
	}

	{
		std::cout << "Right: " << std::endl;
		Parameter *start = probs.less_than_upper_bound(grp);
		Parameter *end = probs.more_than_upper_bound(grp);
		if (!ends) end = nullptr;
		right = chain_and(right, residue_boundary(false, start));
		right = chain_and(right, residue_boundary(true, end));
	}
	std::cout << std::endl;
}

ProblemPrep::ParamSet ProblemPrep::problemParams(PCA::Matrix &m,
                                                     const std::vector<Atom *> 
                                                     &atoms)
{
	ParamSet params;
	SquareSplitter sqsp(m);
	std::vector<int> splits = sqsp.splits();
	std::vector<float> drops = sqsp.drops();
	
	for (size_t i = 0; i < splits.size(); i++)
	{
		int split_idx = splits[i];

		for (int k = -2; k <= 2; k++)
		{
			int include = split_idx + k;
			const Atom *atom = atomFrom(atoms, include);
			addAtomToParams(params, atom, drops[i]);
		}
	}

	return params;
}

ProblemPrep::ParamSet ProblemPrep::filter(const ProblemPrep::ParamSet &other,
                                          int tidx)
{
	ParamSet filtered;
	for (Parameter *const &p : other)
	{
		if (_filter(p, tidx))
		{
			filtered.insert(p);
		}
	}

	return filtered;
}

void ProblemPrep::sort(Parameters &params)
{
	auto sort_by_res = [](const Parameter *a, const Parameter *b)
	{
		if (a->residueId() == b->residueId())
		{
			return a->desc() < b->desc();
		}

		return a->residueId() < b->residueId();
	};
	
	std::sort(params.begin(), params.end(), sort_by_res);
}

void ProblemPrep::processMatrixForTriangle(int tidx, PCA::Matrix &m,
                                           const std::vector<Atom *> &atoms)
{
	ParamSet unfiltered = problemParams(m, atoms);
	_problemsForTriangles[tidx].filtered = unfiltered;
	regroup(_problemsForTriangles[tidx]);

	std::vector<int> pIndices;
	_mapped->point_indices_for_face(tidx, pIndices);
	
	ParamSet filtered = filter(unfiltered, tidx);

	for (const int &p : pIndices)
	{
		_problemsForPoints[p].addSet(filtered);
	}
}

void ProblemPrep::startAssessment()
{
	_problemsForPoints.clear();
	_problemsForTriangles.clear();
	_flexResults.clear();
}
	
void ProblemPrep::finishAssessment()
{
	for (size_t i = 0; i < _mapped->pointCount(); i++)
	{
		regroup(_problemsForPoints[i]);
	}
	
	_all = _specified->parameters();

	getFlexes();
}

float ProblemPrep::updateFlex(Parameter *problem, const std::vector<float> &pos)
{
	auto pair = _specified->bondMappingFor(problem);
	Mapped<Floats> *map = pair.first;

	int tidx = map->face_idx_for_point(pos);
	
	if (tidx < 0)
	{
		return _flexResults[problem];
	}

	float val = _flex(problem, tidx);
	_flexResults[problem] = std::max(val, _flexResults[problem]);
	return val;
}

void ProblemPrep::getFlexes()
{
	_flexResults.clear();
	
	for (Parameter *p : _all)
	{
		auto pair = _specified->bondMappingFor(p);
		Mapped<Floats> *map = pair.first;
		int bondidx = pair.second;
		float min = FLT_MAX; float max = -FLT_MAX;

		for (size_t i = 0; i < map->pointCount(); i++)
		{
			float val = map->get_value(i)[bondidx];
			if (val < min) min = val;
			if (val > max) max = val;
		}

		_flexResults[p] = max - min;
	}
}

void ProblemPrep::regroup(Problems &problems)
{
	ParamSet &src = problems.filtered;
	std::vector<Parameters> &groups = problems.grouped;

	Parameters all;

	for (Parameter *p : src)
	{
		all.push_back(p);
	}
	
	sort(all);

	groups.clear();
	Parameters current;

	for (Parameter *param : all)
	{
		if (current.size() == 0)
		{
			current.push_back(param);
		}
		else if (param->residueId().as_num() -
		         current.back()->residueId().as_num() < 5)
		{
			current.push_back(param);
		}
		else
		{
			groups.push_back(current);
			current.clear();
			current.push_back(param);
		}
	}
	
	if (current.size())
	{
		groups.push_back(current);
	}
}

ProblemPrep::Parameters ProblemPrep::sortedByDescendingFlex()
{
	Parameters ret;
	
	for (Parameter *p : _all)
	{
		ret.push_back(p);
	}

	std::sort(ret.begin(), ret.end(), 
	          [this](Parameter *const &a,
	          Parameter *const &b)
	{
		return this->_flexResults[a] > this->_flexResults[b];
	});
	
	return ret;
}
