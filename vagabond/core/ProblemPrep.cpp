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
#include "ProblemPrep.h"
#include "SpecificNetwork.h"
#include "SquareSplitter.h"
#include "Atom.h"

float check_indices(int tridx, Mapped<float> *const &map)
{
	if (!map) { return false; }
	std::vector<int> pIndices;
	map->point_indices_for_face(tridx, pIndices);

	float min = FLT_MAX; float max = -FLT_MAX;

	float ave = 0;
	for (const int &pidx : pIndices) 
	{
		float next = map->get_value(pidx);
		min = std::min(min, next);
		max = std::max(max, next);
		ave += next;
	}
	
	ave /= (float)pIndices.size();

	return (max - min);
}

ProblemPrep::ProblemPrep(SpecificNetwork *sn, Mapped<float> *mapped)
{
	_specified = sn;
	_mapped = mapped;

	_flex = [sn](Parameter *problem, int tridx) -> float
	{
		Mapped<float> *map = sn->mapForParameter(problem);
		float ave = check_indices(tridx, map);
		return ave;
	};

	std::function<bool(Parameter *, int)> variation;
	variation = [sn](Parameter *problem, int tridx) -> bool
	{
		Mapped<float> *map = sn->mapForParameter(problem);
		float ave = check_indices(tridx, map);
		return (ave > 30);
	};

	_filter = [variation](Parameter *problem, int tridx) -> bool
	{
		return (problem->coversMainChain() && variation(problem, tridx));
	};
}

const Atom *atomFrom( const std::vector<Atom *> &atoms, int idx)
{
	if (idx < 0 || idx >= atoms.size())
	{
		return nullptr;
	}
	
	return atoms.at(idx);
}

void addAtomToParams(ProblemPrep::ParamSet &params, const Atom *atom)
{
	if (atom == nullptr)
	{
		return;
	}

	for (size_t j = 0; j < atom->parameterCount(); j++)
	{
		Parameter *p = atom->parameter(j);
		params.insert(p);
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
                             ProblemPrep::AtomFilter &right)
{
	ProblemPrep::AtomFilter all = [](Atom *const &atom) { return true; };
	left = all; right = all;

	Problems &probs = _problemsForPoints[pidx];
	
	std::cout << "Starting group: " << grp << std::endl;
	for (size_t i = 0; i < probs.grouped[grp].size(); i++)
	{
		std::cout << probs.grouped[grp][i]->residueId() << " ";
		std::cout << probs.grouped[grp][i]->desc() << std::endl;
	}

	{
		std::cout << "Left: " << std::endl;
		Parameter *start = probs.less_than_lower_bound(grp);
		Parameter *end = probs.less_than_upper_bound(grp);
		left = chain_and(left, residue_boundary(false, start));
		left = chain_and(left, residue_boundary(true, end));
	}

	{
		std::cout << "Right: " << std::endl;
		Parameter *start = probs.more_than_lower_bound(grp);
		Parameter *end = probs.more_than_upper_bound(grp);
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
	
	for (size_t i = 0; i < splits.size(); i++)
	{
		int split_idx = splits[i];

		for (int k = -2; k <= 2; k++)
		{
			int include = split_idx + k;
			const Atom *atom = atomFrom(atoms, include);
			addAtomToParams(params, atom);
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
		regroup(_problemsForPoints[p]);
	}
	
	getFlexes();
}

float ProblemPrep::updateFlex(Parameter *problem, const std::vector<float> &pos)
{
	Mapped<float> *map = _specified->mapForParameter(problem);
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
	std::map<int, Problems> &src = _problemsForTriangles;
	
	for (auto it = src.begin(); it != src.end(); it++)
	{
		Problems &probs = it->second;
		
		for (Parameter *p : probs.filtered)
		{
			float val = _flex(p, it->first);
			_flexResults[p] = std::max(val, _flexResults[p]);
		}
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
		}
	}
	
	if (current.size())
	{
		groups.push_back(current);
	}

	/*
	for (Parameters &vec : groups)
	{
		std::cout << "Group of " << vec.size() << std::endl;
		
		for (Parameter *param : vec)
		{
			std::cout << "\t";
			std::cout << param->residueId() << " ";
			std::cout << param->desc() << std::endl;
		}

	}
	*/
}


