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

#ifndef __vagabond__RefinementInfo__
#define __vagabond__RefinementInfo__

#include "StructureModification.h"
#include <vagabond/utils/OpSet.h>
#include <vagabond/c4x/Angular.h>
#include "ResidueTorsion.h"
#include "Residue.h"

template <typename I, typename O> class Task;

class Unit;
class BaseTask;
class Barycentric;
class Warp;

inline std::vector<std::vector<float>> split(const std::vector<float> &vals,
                                             const std::vector<int> &list)
{
	std::vector<std::vector<float>> paramses;
	auto it = vals.begin();

	for (const int &length : list)
	{
		std::vector<float> params;
		params.reserve(length);
		params.insert(params.begin(), it, it + length);
		paramses.push_back(params);
		it = it + length;
	}
	
	return paramses;
}

/** \class RefinementInfo
 *  \brief contains information about how to replicate refinement setup */

namespace Refine
{
	typedef std::function<BaseTask *(StructureModification::Resources &,
	                                 Task<Result, void *> *,
	                                 const std::vector<float> &)> Calculate;

	typedef std::function<BaseTask *(StructureModification::Resources &,
	                                 Task<Result, void *> *)> Submit;
	
	enum Module
	{
		None = 0,
		Warp = 1 << 0,
		Translate = 1 << 1,
		Rotate = 1 << 2,
		ImplicitB = 1 << 3,
		Barycentric = 1 << 4,
	};
	
	struct Calc
	{
		Calculate op; // organises the tasks
		Submit submit; // empty?
		int n_params; // total number of params to handle
	};

	struct Info
	{
		std::vector<Instance *> instances;
		Unit *refiner = nullptr;
		OpSet<Atom *> anchors;
		OpSet<Atom *> all_atoms;
		std::vector<Calc> subunits;
		int samples = 120;
		int master_dims = 3;
		float max_res = -1;
		class Warp *warp = nullptr;
		class Barycentric *barycentric = nullptr;
		Module modules = Refine::None;

		void bind_parameters(const std::vector<float> &vals)
		{
			auto it = vals.begin();

			for (Calc &calc : subunits)
			{
				const int &length = calc.n_params;
				std::vector<float> params;
				params.reserve(length);
				params.insert(params.begin(), it, it + length);
				it = it + length;
				calc.submit = std::bind(calc.op, std::placeholders::_1,
				                        std::placeholders::_2, params);
			}
		}
		
		int total_params()
		{
			int params = 0;
			for (Calc &calc : subunits)
			{
				const int &length = calc.n_params;
				params += length;
			}
			return params;
		}
	};
	
};

#endif
