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

#include "RefinementInfo.h"
#include "FromWarp.h"
#include "AtomGroup.h"
#include "Instance.h"
#include "Sampler.h"
#include "Warp.h"

#include "engine/MapTransferHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/MapSumHandler.h"
#include "BondSequenceHandler.h"
#include "BondCalculator.h"

#include "engine/Task.h"

#include "Translation.h"

using std::placeholders::_1;
using std::placeholders::_2;

FromWarp::FromWarp(Instance *const &instance, Refine::Info &info,
                           Sampler *sampler)
: _info(info), 
_instance(instance),
_sampler(sampler)
{

}

int FromWarp::confParams()
{
	int n = _info.master_dims;
	return n + n * (n + 1) / 2;
}

int FromWarp::transParams()
{
	int n = _info.master_dims;
	return (n + 1) * 3;
}

std::vector<std::vector<float>> split(const std::vector<float> &vals,
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

Refine::Calculate FromWarp::prepareSubmission()
{
	CoordManager manager;
	Sampler *sampler = _sampler;

	rope::GetListFromParameters transform = [sampler](const std::vector<float> &all)
	{
		return sampler->coordsFromParams(all);
	};

	Warp *warp = _info.warp;
	rope::GetFloatFromCoordIdx fetchTorsion;
	fetchTorsion = [warp](const Coord::Get &get, const int &idx)
	{
		return warp->torsionAnglesForCoord()(get, idx);
	};

	manager.setTorsionFetcher(fetchTorsion);
	manager.setDefaultCoordTransform(transform);
	
	std::vector<int> list = {confParams(), transParams()};
	int dims = _info.master_dims;

	return [sampler, dims, manager, list]
	(StructureModification::Resources &resources, 
	 Task<Result, void *> *submit_result,
	 const std::vector<float> &params) 
	-> BaseTask *
	{
		std::vector<std::vector<float>> paramses = split(params, list);

		BaseTask *first_hook = nullptr;

		/* get easy references to resources */
		BondSequenceHandler *const &sequences = resources.sequences;
		MapTransferHandler *const &eleMaps = resources.perElements;
		MapSumHandler *const &sums = resources.summations;
		CorrelationHandler *const &correlation = resources.correlations;

		Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoSuperpose);
		Flag::Extract gets = Flag::Extract(Flag::AtomMap);

		Task<BondSequence *, void *> *letgo = nullptr;

		CalcTask *final_hook = nullptr;

		/* calculation of torsion angle-derived and target-derived
		 * atom positions */
		sequences->calculate(calc, paramses[0], &first_hook, &final_hook, &manager);

		/* on top of this, apply translations */
		rope::IntToCoordGet raw = sampler->rawCoordinates();
		std::vector<float> &trans_params = paramses[1];

		auto add_tr = [dims, trans_params,
		               raw](BondSequence *seq) -> BondSequence *
		{
			Translation translation(dims);
			rope::GetVec3FromIdx get = translation.translate(raw, trans_params);
			seq->addTranslation(get);
			return seq;
		};

		CalcTask *translate = new CalcTask(add_tr, "translate");

		final_hook->follow_with(translate);

		letgo = sequences->extract(gets, submit_result, translate);

		/* Prepare the scratch space for per-element map calculation */
		std::map<std::string, GetEle> eleTasks;

		/* positions coming out of sequence to prepare for per-element maps */
		sequences->positionsForMap(translate, letgo, eleTasks);

		/* updates the scratch space */
		eleMaps->extract(eleTasks);

		/* summation of per-element maps into final real-space map */
		Task<SegmentAddition, AtomMap *> *make_map = nullptr;
		sums->grab_map(eleTasks, submit_result, &make_map);

		/* correlation of real-space map against reference */
		Task<CorrelMapPair, Correlation> *correlate = nullptr;
		correlation->get_correlation(make_map, &correlate);

		/* pop correlation into result */
		correlate->follow_with(submit_result);
		
		return first_hook;
	};
}

void FromWarp::operator()()
{
	AtomGroup *group = _instance->currentAtoms();
	_info.anchors += group->chosenAnchor();
	_info.all_atoms += group->atomVector();
	
	auto full = prepareSubmission();
	Refine::Calc calc = {full, {}, n_params()};

	_info.subunits.push_back(calc);
}

