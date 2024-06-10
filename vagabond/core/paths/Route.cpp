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

#include <algorithm>
#include <stdlib.h>

#include "paths/Route.h"
#include "Polymer.h"
#include "Grapher.h"
#include "ParamSet.h"
#include "Separation.h"
#include "TorsionData.h"
#include "TorsionBasis.h"
#include "BondSequence.h"
#include "BondCalculator.h"
#include "PairwiseDeviations.h"
#include "EnergyTorsions.h"
#include "GradientTerm.h"
#include "paths/BundleBonds.h"
#include "function_typedefs.h"

#include "matrix_functions.h"
#include "engine/Tasks.h"
#include "engine/Task.h"
#include "BondSequenceHandler.h"

Route::Route(Instance *from, Instance *to, const RTAngles &list)
{
	srand(time(NULL));
	setInstance(from);
	_endInstance = to;
	_source = list;
	
	addLinkedInstances(from, to);
}

Route::Route(const RTAngles &list)
{
	srand(time(NULL));
	_source = list;
}

void Route::addLinkedInstances(Instance *from, Instance *to)
{
	_pairs.push_back({from, to});
	if (_instance == nullptr)
	{
		_instance = from;
		_endInstance = to;
	}
	from->load();
}

AtomGroup *Route::all_atoms()
{
	AtomGroup *grp = new AtomGroup();
	
	for (InstancePair &pair : _pairs)
	{
		AtomGroup *next = pair.start->currentAtoms();
		next->recalculate();
		grp->add(next);
	}

	return grp;
}

Route::~Route()
{
	deleteHelpers();
	
}

void Route::setup()
{
	if (_setup)
	{
		return;
	}

	if (_source.size() == 0 && motionCount() == 0)
	{
		throw std::runtime_error("No destination or prior set for route");
	}

	prepareResources();
	_setup = true;
}

float Route::submitJobAndRetrieve(float frac, bool show)
{
	clearTickets();

	submitToShow(frac);

	retrieve();
	
	float ret = _point2Score.begin()->second.deviations;

	return ret;
}

GradientPath *Route::submitGradients(const CalcOptions &options, int order,
                                     const ValidateParam &validate,
                                     BondSequenceHandler *handler)
{
	if (handler == nullptr) handler = _hydrogenFreeSequences;
	
	int steps = (order + 1) * 2;

	_gradientBin.holdHorses();

	PairwiseDeviations *pw = _helpers[handler].pw;
	Separation *sep = _helpers[handler].sep;

	// zero = all calculations
	Task<GradientPath, void *> *big_submission = _gradientBin.actOfSubmission(0);
	Flag::Calc calc = Flag::Calc(Flag::DoTorsions);

	std::vector<int> indices;
	std::vector<int> motion_idxs;

	std::vector<AtomBlock> &blocks = handler->sequence()->blocks();
	TorsionBasis *basis = handler->torsionBasis();

	bool clash = (options & VdWClashes);
	if (clash)
	{
		throw std::runtime_error("Not supporting gradients for clash right now");
	}

	for (size_t i = 0; i < blocks.size(); i++)
	{
		if (blocks[i].torsion_idx < 0) { continue; }
		int tidx = blocks[i].torsion_idx;
		if (blocks[i].atom->elementSymbol() == "H")
		{
			continue;
		}

		Parameter *p = basis->parameter(tidx);
		int pidx = indexOfParameter(p);
		if (validate && !validate(pidx))
		{
			continue;
		}
		if (p->isConstrained())
		{
			continue;
		}
		if (_motionFilter && !_motionFilter(pidx))
		{
			continue;
		}

		indices.push_back(i); // pointer to block index (atom)
		motion_idxs.push_back(pidx); // pointer to motion or parameter
	}

	big_submission->input.grads.resize(indices.size());
	big_submission->input.motion_idxs = motion_idxs;
	big_submission->at_least = (steps - 1) * indices.size();

	for (size_t i = 0; i < steps; i++)
	{
		if (i == 0) continue;

		float frac = i / (float)steps;

		BaseTask *first_hook = nullptr;
		CalcTask *final_hook = nullptr;
		handler->calculate(calc, {frac}, &first_hook, &final_hook);
		Task<BondSequence *, void *> *let_go = handler->letGo();
		
		for (int j = 0; j < indices.size(); j++)
		{
			int b_idx = indices[j];
			Parameter *p = basis->parameter(blocks[b_idx].torsion_idx);
			int g_idx = j;
			auto momentum_term = [sep, order, frac, g_idx, b_idx, p, pw]
			(BondSequence *seq) -> GradientTerm
			{
				GradientTerm term(order, frac, g_idx, b_idx, p);
				term.momentum(seq, pw, sep);
				return term;
			};

			auto make_term = 
			(new Task<BondSequence *, GradientTerm> (momentum_term, 
                                         "momentum gradient"));

			final_hook->follow_with(make_term);
			final_hook->follow_with(let_go);
			make_term->follow_with(big_submission);
			make_term->must_complete_before(let_go);
		}

		_resources.tasks->addTask(first_hook);
	}

	_gradientBin.releaseHorses();
	GradientPath *r = _gradientBin.acquireObject();
	
	return r;
}

struct TaskInfo
{
	CalcTask *final_hook;
	Task<BondSequence *, void *> *let_go;
	Task<BundleBonds *, void *> *delete_bundle;
	Task<BundleBonds *, BundleBonds *> *bundle_hook;
};

void Route::prepareAlignment()
{
	if (_noncovs)
	{
		auto alignment = _noncovs->align_task();
		_postCalcTasks.push_back(alignment);
	}
}

void Route::submitValue(const CalcOptions &options, int steps,
                        BondSequenceHandler *handler)
{
	bool pairwise = (options & Pairwise);
	bool hydrogens = !(options & NoHydrogens);
	bool torsion_energies = (options & TorsionEnergies);
	bool vdw_clashes = (options & VdWClashes);
	bool per_residue = (options & PerResidue && vdw_clashes);

	Flag::Extract gets = pairwise ? Flag::NoExtract : Flag::Deviation;

	std::vector<BaseTask *> firsts;
	std::map<int, TaskInfo> frac_tasks;

	/* get easy references to resources */
	BondSequenceHandler *sequences = (hydrogens ? _resources.sequences : 
	                                  _hydrogenFreeSequences);

	BondCalculator *const &calculator = _resources.calculator;
	calculator->reset();
	Task<Result, void *> *submit_result = calculator->actOfSubmission(0);

	// dependent on sequences, steps, firsts, frac_tasks, extra tasks like noncovs
	for (int i = 0; i < steps; i++)
	{
		float frac = i / (float)steps;

		BaseTask *first_hook = nullptr;
		CalcTask *final_hook = nullptr;

		/* this final task returns the result to the pool to collect later */

		Flag::Calc calc = Flag::Calc(Flag::DoTorsions);
		if (!pairwise)
		{
			calc = Flag::Calc(Flag::DoSuperpose | calc);
		}

		/* calculation of torsion angle-derived and target-derived
		 * atom positions */
		sequences->calculate(calc, {frac}, &first_hook, &final_hook);

		for (auto &task : _postCalcTasks)
		{
			CalcTask *job = new CalcTask(task, "post-calc task");
			final_hook->follow_with(job);
			final_hook = job;
		}

		Task<Result, void *> *sr = pairwise ? nullptr : submit_result;
		
		// only put deviations together for non-pairwise calculation
		Task<BondSequence *, void *> *let = 
		sequences->extract(gets, sr, final_hook);

		frac_tasks[i].final_hook = final_hook;
		frac_tasks[i].let_go = let;
		
		firsts.push_back(first_hook);
	}

	Task<Result, void *> *first_only = nullptr;
	Task<ByResidueResult, void *> *by_residue = nullptr;

	if (pairwise)
	{
		first_only = calculator->actOfSubmission(1);

		if (per_residue)
		{
			by_residue = _perResBin.actOfSubmission(0);
			_perResBin.holdHorses();
		}
	}
	
	bool uses_bundles = vdw_clashes;

	int n_bundles = 0;
	for (int i = 0; i < steps - 1 && pairwise; i++)
	{
		TaskInfo &info = frac_tasks[i];

		float frac = i / (float)steps;

		if (uses_bundles)
		{
			BundleBonds *bbs = new BundleBonds(sequences->sequence(), frac);

			if (!hydrogens) bbs->addVdWRadius(0.4);

			auto bundle_hook = [](BundleBonds *bbs) -> BundleBonds *
			{
				return bbs;
			};

			auto del_bundle = [](BundleBonds *bbs) -> void *
			{
				delete bbs;
				return nullptr;
			};

			info.delete_bundle =
			new Task<BundleBonds *, void *>(del_bundle, "delete bundle "
			                                + std::to_string(i));

			info.bundle_hook = 
			new Task<BundleBonds *, BundleBonds *>(bundle_hook, "bundle hook "
			                                       + std::to_string(i));

			for (int j = -1; j <= 2; j++)
			{
				int n = i + j;
				if (n < 0 || n >= steps)
				{
					continue;
				}

				TaskInfo &local_info = frac_tasks[n];
				auto bundle_seq = [j, bbs](BondSequence *seq) -> BundleBonds *
				{
					*bbs += {j + 1, seq};
					return bbs;
				};

				n_bundles++;
				auto *bundle = new Task<BondSequence *, BundleBonds *>
				(bundle_seq, "bundle sequence " + std::to_string(n_bundles));

				local_info.final_hook->follow_with(bundle);
				bundle->follow_with(info.bundle_hook);
				bundle->must_complete_before(local_info.let_go);
			}

			submit_result->must_complete_before(info.delete_bundle);
			first_only->must_complete_before(info.delete_bundle);
			info.bundle_hook->follow_with(info.delete_bundle);
		}

		auto setup_submit_hooks =
		[&submit_result, &first_only] <typename SpecificTask>
		(SpecificTask *task, int i)
		{
			task->follow_with(submit_result);
			if (i == 0)
			{
				task->follow_with(first_only);
			}
		};

		PairwiseDeviations *chosen = _helpers[sequences].pw;

		std::set<ResidueId> active_ids = 
		doingSides() ? _ids : std::set<ResidueId>();

		if (!vdw_clashes)
		{
			Task<BondSequence *, Deviation> *task = nullptr;
			task = chosen->momentum_task(frac, active_ids);
			info.final_hook->follow_with(task);
			task->must_complete_before(info.let_go);
			setup_submit_hooks(task, i);
		}
		else
		{
			Task<BundleBonds *, ActivationEnergy> *task = nullptr;
			task = chosen->bundle_clash(active_ids);
			info.bundle_hook->follow_with(task);
			setup_submit_hooks(task, i);
		}


		if (per_residue)
		{
			std::set<ResidueId> residues = chosen->residues();

			for (const ResidueId &id : residues)
			{
				if (_ids.size() > 0 && _ids.count(id) == 0)
				{
					continue;
				}
				Task<BundleBonds *, ActivationEnergy> *task = nullptr;
				task = chosen->bundle_clash({id});
				info.bundle_hook->follow_with(task);
				task->must_complete_before(info.delete_bundle);

				Task<BondSequence *, ActivationEnergy> *engy = nullptr;
				if (torsion_energies && hydrogens)
				{
					EnergyTorsions *chosen = _helpers[sequences].et;
					engy = chosen->energy_task({id}, frac);
					info.final_hook->follow_with(engy);
					engy->must_complete_before(info.let_go);
				}

				auto convert = [id, steps](const ActivationEnergy &ae) 
				-> SingleResidueResult
				{
					return {id, ae.value / steps};
				};

				auto *vdw_conv =
				new Task<ActivationEnergy, SingleResidueResult>(convert,
				                                                "convert vdw to single residue result");

				task->follow_with(vdw_conv);
				vdw_conv->follow_with(by_residue);

				if (engy)
				{
					auto *engy_conv =
					new Task<ActivationEnergy, 
					SingleResidueResult>(convert,
					                     "convert energy to single");

					engy->follow_with(engy_conv);
					engy_conv->follow_with(by_residue);
				}
			}
		}

		// torsion energies
		if (torsion_energies && hydrogens && !per_residue)
		{
			EnergyTorsions *chosen = _helpers[sequences].et;
			Task<BondSequence *, ActivationEnergy> *task = nullptr;
			task = chosen->energy_task(active_ids, frac);
			info.final_hook->follow_with(task);
			task->must_complete_before(info.let_go);
			setup_submit_hooks(task, i);
		}
	}

	_ticket2Point[0] = 0;
	_point2Score[0] = Score{};

	_ticket2Point[1] = 1;
	_point2Score[1] = Score{};
	
	for (BaseTask *t : firsts)
	{
		_resources.tasks->addTask(t);
	}

//	calculator->releaseHorses();
}

void Route::submitToShow(float frac)
{
	Flag::Extract gets = Flag::AtomVector;

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->actOfSubmission(0);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoSuperpose);

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, {frac}, &first_hook, &final_hook);
	
	for (auto &task : _postCalcTasks)
	{
		CalcTask *job = new CalcTask(task, "post-calc task");
		final_hook->follow_with(job);
		final_hook = job;
	}

	Task<BondSequence *, void *> *let = 
	sequences->extract(gets, submit_result, final_hook);

	_ticket2Point[0] = 0;
	_point2Score[0] = Score{};
	
	_resources.tasks->addTask(first_hook);
}

const Grapher &Route::grapher() const
{
	const Grapher &g = _resources.sequences->grapher();
	return g;
}

AtomGraph *Route::grapherForTorsionIndex(int idx)
{
	AtomGraph *ag = grapher().graph(parameter(idx));

	return ag;
}

void Route::setFlips(std::vector<int> &idxs, std::vector<int> &fs)
{
	for (size_t j = 0; j < idxs.size(); j++)
	{
		setFlip(idxs[j], fs[j]);
	}
}

void Route::getParametersFromBasis(const MakeMotion &make_mot)
{
	ParamSet missing;

	std::vector<Motion> tmp_motions;
	std::vector<ResidueTorsion> torsions;

	TorsionBasis *basis = _resources.sequences->torsionBasis();

	for (size_t i = 0; i < basis->parameterCount(); i++)
	{
		Parameter *p = basis->parameter(i);
		ResidueTorsion rt{};
		tmp_motions.push_back(make_mot(p, rt));
		torsions.push_back(rt);
	}

	_motions = RTMotion::motions_from(torsions, tmp_motions);
	_parameter2Idx.clear();
	
	for (int i = 0; i < motionCount(); i++)
	{
		Parameter *param = parameter(i);
		if (param)
		{
			_parameter2Idx[param] = i;
		}
	}
}

void Route::prepareParameters()
{
	auto instance_for_param = [this](Parameter *const &param) -> Instance *
	{
		Atom *atom = param->owningAtom();

		for (const InstancePair &pair : _pairs)
		{
			if (pair.start->atomBelongsToInstance(atom))
			{
				return pair.start;
			}
		}
		
		return nullptr;
	};

	auto make_motion = [this, instance_for_param]
	(Parameter *const &param, ResidueTorsion &rt)
	{
		Instance *inst = instance_for_param(param);
		rt = ResidueTorsion(param);
		rt.attachToInstance(inst);

		// index may be in existing motions, or in the source torsions
		int mot_idx = _motions.indexOfHeader(rt);
		int src_idx = _source.indexOfHeader(rt);
		
		Motion mt = {WayPoints(_order), false, 0};

		if (mot_idx >= 0)
		{
			Motion &target = _motions.storage(mot_idx);
			mt = target;
		}

		// if we have a more up-to-date torsion angle then we should use that,
		// but take into account 360 degree flips
		if (src_idx >= 0 && mot_idx < 0)
		{
			mt.angle = _source.storage(src_idx);
		}
		else if (src_idx >= 0 && mot_idx >= 0)
		{
			float torsion = _source.storage(src_idx);
			float last = mt.angle;
			while (torsion < last - 180.f) torsion += 360.f;
			while (torsion >= last + 180.f) torsion -= 360.f;

			mt.angle = torsion;
		}
		
		if (mt.angle != mt.angle)
		{
			mt.angle = 0; // usually due to mismatched sequence
		}
		
		return mt;
	};

	getParametersFromBasis(make_motion);
}

void Route::prepareDestination()
{
	if (_source.size() == 0 && _motions.size() == 0)
	{
		throw std::runtime_error("Raw destination not set, nor motions for "\
		                         "destination");
	}
	
	prepareParameters();
}

int Route::indexOfParameter(Parameter *t)
{
	if (_parameter2Idx.count(t) == 0)
	{
		return -1;
	}

	return _parameter2Idx.at(t);
}

float Route::getTorsionAngle(int i)
{
	return  motion(i).workingAngle();
}

std::vector<ResidueTorsion> Route::residueTorsions()
{
	std::vector<ResidueTorsion> rts;
	for (size_t i = 0; i < motionCount(); i++)
	{
		rts.push_back(residueTorsion(i));
	}

	return rts;
}

void Route::prepareResources()
{
	_resources.allocateMinimum(_threads);

	/* prepare separate sequences for main-chain only */
	_hydrogenFreeSequences = new BondSequenceHandler(_threads);

	for (const InstancePair &pair : _pairs)
	{
		AtomGroup *group = pair.start->currentAtoms();
		std::vector<AtomGroup *> subsets = group->connectedGroups();
		for (AtomGroup *subset : subsets)
		{
			Atom *anchor = subset->chosenAnchor();
			_resources.sequences->addAnchorExtension(anchor);
			_hydrogenFreeSequences->addAnchorExtension(anchor);
		}
	}

	_hydrogenFreeSequences->setIgnoreHydrogens(true);
	_hydrogenFreeSequences->setAtomFilter(rope::atom_is_not_hydrogen());
	_hydrogenFreeSequences->setup();
	_hydrogenFreeSequences->prepareSequences();

	_resources.sequences->setup();
	_resources.sequences->prepareSequences();

	updateAtomFetch(_resources.sequences);
	updateAtomFetch(_hydrogenFreeSequences);
	
	if (_noncovs)
	{
		_noncovs->provideSequence(_resources.sequences->sequence());
	}
}

void Route::deleteHelpers()
{
	for (auto it = _helpers.begin(); it != _helpers.end(); it++)
	{
		delete it->second.pw;
		delete it->second.et;
		delete it->second.sep;
	}

	_helpers.clear();
}

void setup_helpers(Route::Helpers &helpers, BondSequence *seq, float distance)
{
	Separation *sep = new Separation(seq);
	auto pw = new PairwiseDeviations(seq, distance, sep);

	helpers.pw = pw;
	helpers.sep = sep;

}

void Route::prepareEnergyTerms()
{
	deleteHelpers();

	{
		BondSequence *seq = _hydrogenFreeSequences->sequence();
		setup_helpers(_helpers[_hydrogenFreeSequences], 
		              seq, _maxMomentumDistance);
	}

	if (_noncovs)
	{
		BondSequence *seq = _resources.sequences->sequence();
		setup_helpers(_helpers[_resources.sequences], seq, _maxClashDistance);
	}
	else
	{
		auto pw = new PairwiseDeviations(_resources.sequences->sequence(),
		                                 _maxClashDistance);
		_helpers[_resources.sequences].pw = pw;
	}

	{
		auto lookup = [this](Parameter *t)
		{
			return indexOfParameter(t);
		};
		auto et = new EnergyTorsions(_resources.sequences->sequence(),
		                             motions(), lookup);
		_helpers[_resources.sequences].et = et;
	}

}

void Route::updateAtomFetch(BondSequenceHandler *const &handler)
{
	const std::vector<AtomBlock> &blocks = 
	handler->sequence()->blocks();

	CoordManager *manager = handler->manager();
	manager->setAtomFetcher(AtomBlock::prepareMovingTargets(blocks));
}

void Route::clearCustomisation()
{
	_finish = false;

	for (size_t i = 0; i < motionCount(); i++)
	{
		motion(i).wp = WayPoints(_order);
	}

	{
		delete _helpers[_resources.sequences].pw;
		auto pw = new PairwiseDeviations(_resources.sequences->sequence(),
		                                 _maxClashDistance);
		_helpers[_resources.sequences].pw = pw;
	}
	{
		delete _helpers[_hydrogenFreeSequences].pw;
		auto pw = new PairwiseDeviations(_hydrogenFreeSequences->sequence(),
		                                 _maxClashDistance);
		_helpers[_hydrogenFreeSequences].pw = pw;
	}
	
	_jobLevel = 0;
	_repelCount = 0;
	unlockAll();
	_hash = ""; setHash();
}

void Route::setHash(const std::string &hash)
{
	if (_hash.length() == 0 && hash.length() > 0)
	{
		_hash = hash;
	}
	else if (_hash.length() == 0)
	{
		unsigned int rand_num = rand();
		char str[65];
		sprintf(str, "%x", rand_num);

		std::string hash; hash += str;
		_hash = hash;
		std::cout << "New hash: " << _hash << std::endl;
	}
}

void Route::calculate(Route *me)
{
	try
	{
		me->_calculating = true;
		me->doCalculations();
		me->_calculating = false;
	}
	catch (const std::runtime_error &error)
	{
		std::string *str = new std::string(error.what());
		me->sendResponse("error", str);
	}
}

void Route::unlockAll()
{
	for (size_t i = 0; i < motionCount(); i++)
	{
		motion(i).locked = false;
	}

}
