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

#ifndef __vagabond__ProblemPrep__
#define __vagabond__ProblemPrep__

#include <vagabond/utils/svd/PCA.h>
#include <map>
#include <set>
class Atom;
class Parameter;
class SpecificNetwork;
template <typename Type> class Mapped;

class ProblemPrep
{
public:
	ProblemPrep(SpecificNetwork *sn, Mapped<float> *mapped);

	typedef std::function<bool(Atom *const &atom)> AtomFilter;
	typedef std::vector<Parameter *> Parameters;
	typedef std::set<Parameter *> ParamSet;

	void processMatrixForTriangle(int idx, PCA::Matrix &m,
	                              const std::vector<Atom *> &atoms);
	

	void setFilters(int pidx, int grp, AtomFilter &left, AtomFilter &right);

	Parameters paramsForPoint(int pidx, int grp);
	Parameters paramsForTriangle(int tidx, int grp);
	size_t groupCountForPoint(int pidx);
	size_t groupCountForTriangle(int pidx);
	
	float flexForParameter(Parameter *p)
	{
		return _flexResults[p];
	}
	
	std::function<float(int &)> flipFunction(Parameter *param, int pidx);

	float updateFlex(Parameter *problem, const std::vector<float> &pos);
private:
	void getFlexes();

	ParamSet problemParams(PCA::Matrix &m, const std::vector<Atom *> &atoms);
	ParamSet filter(const ParamSet &other, int tidx);
	void sort(Parameters &params);

	struct Problems
	{
		ParamSet filtered;
		std::vector<Parameters> grouped;
		
		void addSet(const ParamSet &set)
		{
			for (Parameter *p : set)
			{
				filtered.insert(p);
			}
		}

		Parameter *more_than_lower_bound(int pidx)
		{
			return grouped[pidx].back();
		}
		
		Parameter *more_than_upper_bound(int pidx)
		{
			if (pidx >= grouped.size() - 1) return nullptr;
			return grouped[pidx + 1].front();
		}
		
		Parameter *less_than_upper_bound(int pidx)
		{
			return grouped[pidx].front();
		}
		
		Parameter *less_than_lower_bound(int pidx)
		{
			if (pidx <= 0) return nullptr;
			return grouped[pidx - 1].back();
		}
	};
	
	void regroup(Problems &problems);

	std::map<int, Problems> _problemsForPoints;
	std::map<int, Problems> _problemsForTriangles;

	SpecificNetwork *_specified = nullptr;
	Mapped<float> *_mapped = nullptr;

	std::function<bool(Parameter *, int)> _filter;
	std::function<float(Parameter *, int)> _flex;
	
	std::map<Parameter *, float> _flexResults;
};

#endif
