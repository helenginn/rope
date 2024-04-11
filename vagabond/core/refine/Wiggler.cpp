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

#include "Wiggler.h"

#include "RefinementInfo.h"
#include "AtomGroup.h"
#include "Instance.h"
#include "Sampler.h"
#include "Barycentric.h"
#include "Warp.h"

#include "grids/AnisoMap.h"
#include "engine/MapTransferHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/ImplicitBHandler.h"
#include "engine/MapSumHandler.h"
#include "BondSequenceHandler.h"
#include "BondCalculator.h"

#include "engine/Task.h"

#include "Translation.h"
#include "Rotation.h"

#define FIDUCIAL_COUNT (5)

Wiggler::Wiggler(Refine::Info &info, Sampler *sampler)
: _info(info), _sampler(sampler)
{

}

AtomGroup *Wiggler::group()
{
	AtomGroup *group = new AtomGroup();
	for (Instance *inst : _info.instances)
	{
		group->add(inst->currentAtoms());
	}

	return group;
}

CoordManager coordinateManagement(Wiggler *wiggle)
{
	CoordManager manager;
	Sampler *sampler = wiggle->_sampler;

	rope::GetListFromParameters transform = [sampler](const std::vector<float> &all)
	{
		return sampler->coordsFromParams(all);
	};

	manager.setDefaultCoordTransform(transform);

	rope::GetFloatFromCoordIdx fetchTorsion;
	Warp *warp = wiggle->_info.warp;
	
	if (wiggle->_modules & Refine::Warp)
	{
		fetchTorsion = [warp](const Coord::Get &get, const int &idx)
		{
			return warp->torsionAnglesForCoord()(get, idx);
		};
	}
	else
	{
		fetchTorsion = [](const Coord::Get &get, const int &idx)
		{
			return 0.f;
		};
	}

	manager.setTorsionFetcher(fetchTorsion);

	return manager;
}

void scale(std::vector<float> &vals, float sc)
{
	for (float &f : vals)
	{
		f *= sc;
	}
}

Refine::Calculate Wiggler::prepareSubmission()
{
	AtomGroup *grp = group();
	CoordManager manager = coordinateManagement(this);
	Sampler *sampler = _sampler;
	std::vector<int> list = chopped_params();
	int dims = _info.master_dims;
	int warp_dims = _info.warp ? _info.warp->numAxes() : 0;
	Refine::Module modules = _modules;
	glm::vec3 centre = grp->initialCentre();
	bool &superpose = _superpose;
	
	Barycentric *bary = nullptr;
	if (modules & Refine::Barycentric)
	{
		_info.barycentric = new Barycentric(warp_dims, FIDUCIAL_COUNT);
		bary = _info.barycentric;
	}

	return [sampler, dims, manager, list, modules, centre, superpose, bary]
	(StructureModification::Resources &resources, 
	 Task<Result, void *> *submit_result,
	 const std::vector<float> &params) 
	-> BaseTask *
	{
		std::vector<std::vector<float>> paramses = split(params, list);
		paramses.resize(5);

		BaseTask *first_hook = nullptr;

		/* get easy references to resources */
		BondSequenceHandler *const &sequences = resources.sequences;
		MapTransferHandler *const &eleMaps = resources.perElements;
		MapSumHandler *const &sums = resources.summations;
		ImplicitBHandler *const &implicits = resources.implicits;
		CorrelationHandler *const &correlation = resources.correlations;

		Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoSuperpose);
		if (!superpose)
		{
			calc = Flag::DoTorsions;
		}

		Flag::Extract gets = Flag::Extract(Flag::AtomMap);

		Task<BondSequence *, void *> *letgo = nullptr;

		CalcTask *final_hook = nullptr;

		const rope::GetFloatFromCoordIdx &toTorsions = manager.defaultTorsionFetcher();
		
		/*
		if (modules & Refine::Barycentric)
		{
			std::vector<float> baryparams = paramses[4];
			rope::ModifyCoordGet modify = bary->convert(baryparams);

			auto tmp = [modify, toTorsions](const Coord::Get &get, const int &idx)
			{
				const Coord::Get modified = modify(get);
				return toTorsions(modified, idx);
			};
			
			toTorsions = tmp;
		}
		*/

		/* calculation of torsion angle-derived and target-derived
		 * atom positions */
		sequences->calculate(calc, paramses[0], &first_hook, &final_hook, 
		                     &manager/*, toTorsions*/);

		if (modules & Refine::Rotate)
		{
			/* on top of this, apply translations */
			rope::IntToCoordGet raw = sampler->rawCoordinates();
			std::vector<float> &rot_params = paramses[2];

			auto add_rot = [dims, rot_params, centre, raw]
			(BondSequence *seq) -> BondSequence *
			{
				Rotation rotation(dims, centre);
				rope::GetVec3FromIdx get = rotation.rotate(raw, rot_params);
				seq->addOffset(get);
				return seq;
			};

			CalcTask *rotate = new CalcTask(add_rot, "rotate");

			final_hook->follow_with(rotate);
			final_hook = rotate;
		}

		if (modules & Refine::Translate)
		{
			/* on top of this, apply translations */
			rope::IntToCoordGet raw = sampler->rawCoordinates();
			std::vector<float> &trans_params = paramses[1];

			auto add_tr = [dims, trans_params,
			               raw](BondSequence *seq) -> BondSequence *
			{
				Translation translation(dims);
				rope::GetVec3FromIdx get = translation.translate(raw, trans_params);
				seq->addOffset(get);
				return seq;
			};

			CalcTask *translate = new CalcTask(add_tr, "translate");

			final_hook->follow_with(translate);
			final_hook = translate;
		}

		letgo = sequences->extract(gets, submit_result, final_hook);

		/* Prepare the scratch space for per-element map calculation */
		std::map<std::string, GetEle> eleTasks;

		/* positions coming out of sequence to prepare for per-element maps */
		sequences->positionsForMap(final_hook, letgo, eleTasks);

		/* updates the scratch space */
		eleMaps->extract(eleTasks);

		/* summation of per-element maps into final real-space map */
		Task<SegmentAddition, AtomMap *> *make_map = nullptr;
		sums->grab_map(eleTasks, submit_result, &make_map);

		/* correlation of real-space map against reference */
		Task<CorrelMapPair, Correlation> *correlate = nullptr;
		Task<AtomMap *, CorrelMapPair> *grab_correl = nullptr;
		correlation->get_correlation(nullptr, &correlate, &grab_correl);

		if (modules & Refine::ImplicitB)
		{
			/* apply additional anisotropic B to whole unit */

			auto get_implicit = [implicits](void *, bool *success) -> AnisoMap *
			{
				AnisoMap *am = implicits->acquireAnisoMapIfAvailable();
				*success = (am != nullptr);
				return am;
			};

			auto *acquire = new FailableTask<void *, AnisoMap *>(get_implicit);

			make_map->must_complete_before(acquire);
			std::vector<float> anisos = paramses[3];

			auto add_implicit = [anisos](ApplyAniso aa) -> AtomMap *
			{
				aa.aniso->setBs(anisos);
				aa.aniso->applyToMap(aa.map);
				return aa.map;
			};

			auto *apply_aniso = new Task<ApplyAniso, AtomMap *>(add_implicit);

			acquire->follow_with(apply_aniso);
			make_map->follow_with(apply_aniso);

			auto free_implicit = [implicits](AnisoMap *aniso) -> void *
			{
				implicits->returnAnisoMap(aniso);
				return nullptr;
			};

			auto *free_aniso = new Task<AnisoMap *, void *>(free_implicit);
			apply_aniso->must_complete_before(free_aniso);
			acquire->follow_with(free_aniso);
			apply_aniso->follow_with(grab_correl);
		}
		else
		{
			make_map->follow_with(grab_correl);
		}

		/* pop correlation into result */
		correlate->follow_with(submit_result);

		return first_hook;
	};
}

int Wiggler::confParams()
{
	int n = _info.master_dims;
	return n + n * (n + 1) / 2;
}

int Wiggler::transParams()
{
	int n = _info.master_dims;
	return (n + 1) * 3;
}

int Wiggler::baryParams()
{
	int n = FIDUCIAL_COUNT;
	return (n) * _info.warp->numAxes();
}

int Wiggler::rotParams()
{
	int n = _info.master_dims;
	return (n) * 4;
}

int implicitParams()
{
	return 6;
}

std::vector<int> Wiggler::chopped_params()
{
	std::vector<int> list;
	
	list.push_back(_modules & Refine::Warp ? confParams() : 0);
	list.push_back(_modules & Refine::Translate ? transParams() : 0);
	list.push_back(_modules & Refine::Rotate ? rotParams() : 0);
	list.push_back(_modules & Refine::ImplicitB ? implicitParams() : 0);
	list.push_back(_modules & Refine::Barycentric ? baryParams() : 0);
	
	return list;
}

int Wiggler::n_params()
{
	std::vector<int> list = chopped_params();
	int total = 0;

	std::cout << "list: ";
	for (int &p : list)
	{
		std::cout << p << ", ";
		total += p;
	}
	std::cout << std::endl;
	
	return total;
}

void Wiggler::operator()()
{
	for (Instance *inst : _info.instances)
	{
		AtomGroup *group = inst->currentAtoms();
		
		std::vector<AtomGroup *> connected = group->connectedGroups();
		
		for (AtomGroup *subgroup : connected)
		{
			_info.anchors += subgroup->chosenAnchor();
		}
	}

	_info.all_atoms += group()->atomVector();
	
	auto full = prepareSubmission();
	Refine::Calc calc = {full, {}, n_params()};

	_info.subunits.push_back(calc);
}

