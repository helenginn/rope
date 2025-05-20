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

#include "SubmissionHelp.h"
#include "engine/Tasks.h"
#include "engine/Task.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "PairwiseDeviations.h"
#include "BondCalculator.h"
#include "Route.h"

SubmissionHelp::SubmissionHelp(Route *route, const CalcOptions &options)
{
	_route = route;
	_options = options;
}

BondSequenceHandler *SubmissionHelp::sequences()
{
	if (_sequences == nullptr)
	{
		if (_options & NoHydrogens)
		{
			_sequences = _route->_hydrogenFreeSequences;
		}
		else
		{
			_sequences = _route->_resources.sequences;
		}
	}
	
	return _sequences;
}

PairwiseDeviations *SubmissionHelp::chosenCache()
{
	return _route->_helpers[sequences()].pairwise.acquire();
}

EnergyTorsions *SubmissionHelp::chosenEnergyTorsions()
{
	return _route->_helpers[sequences()].et;
}

Bin<ResultBy<ResidueId>> &SubmissionHelp::perResidueBin()
{
	return _route->_perResBin;
}

void SubmissionHelp::prepareBinForScoreResult()
{
	BondCalculator *const &calculator = _route->_resources.calculator;
	calculator->reset();
	_submit_result = calculator->actOfSubmission(0);
	
	if (_options & Pairwise)
	{
		_activation_energy_ref_submit = calculator->actOfSubmission(1);

	}
	calculator->holdHorses();
	
	// if calculation occurs per-residue then we need to set up the job 
	// and bin to handle it.
	if (_options & PerResidue)
	{
		submitResidueResult = perResidueBin().actOfSubmission(0);
		// do not output results until fully prepared!
		perResidueBin().holdHorses(); 
	}
}

void SubmissionHelp::calculationExtractionFlags(Flag::Calc *calc,
                                                Flag::Extract *extract)
{
	*calc = Flag::Calc(Flag::DoTorsions);
	*extract = Flag::NoExtract;

	if (!(_options & Pairwise))
	{
		*calc = Flag::Calc(Flag::DoSuperpose | *calc);
		*extract = Flag::Deviation;
	}
	
	if (_acquirePositions)
	{
		*extract = Flag::Extract(Flag::AtomVector | *extract);
	}
}

std::vector<std::function<BondSequence *(BondSequence *)>> 
SubmissionHelp::extraTasks(const float &frac)
{
	std::vector<std::function<BondSequence *(BondSequence *)>> ret;
	if (_route->_noncovs && _route->_noncovs->ready())
	{
		auto alignment = _route->_noncovs->align_task(frac);
		ret.push_back(alignment);
	}
	return ret;
}

void SubmissionHelp::applyPostCalcTasks(CalcTask *&hook, const float &frac)
{
	auto tasks = extraTasks(frac);
	for (auto &task : tasks)
	{
		CalcTask *job = new CalcTask(task, "post-calc task");
		hook->follow_with(job);
		hook = job;
	}
}


void SubmissionHelp::torsionPositionCalculation()
{
	for (int i = 0; i < _steps; i++)
	{
		float frac = (_steps == 1 ? _frac : i / (float)_steps);

		Flag::Calc calc{};
		Flag::Extract extract{};
		calculationExtractionFlags(&calc, &extract);

		BaseTask *first_hook = nullptr;
		CalcTask *final_hook = nullptr;

		/* calculation of torsion angle-derived and target-derived
		 * atom positions */
		sequences()->calculate(calc, {frac}, &first_hook, &final_hook);

		applyPostCalcTasks(final_hook, frac);

		// only put deviations together for non-pairwise calculation as we
		// are then done.
		Task<Result, void *> *sr = nullptr;
		sr = _options & Pairwise ? nullptr : _submit_result;
		
		Task<BondSequence *, void *> *let = 
		sequences()->extract(extract, sr, final_hook);
		
		if (_atom)
		{
			auto get_position = [this](BondSequence *const &seq)
			-> AtomPosList *
			{
				for (int i = 0; i < seq->blockCount(); i++)
				{
					if (seq->blocks()[i].atom == _atom)
					{
						AtomPosList *apl = new AtomPosList(1);
						(*apl)[0].atom = _atom;
						(*apl)[0].wp.ave = seq->blocks()[i].my_position();
						return apl;
					}
				}
				return new AtomPosList(0);
			};

			auto *get_atom = 
			new Task<BondSequence *, AtomPosList *>(get_position, "get atom");

			final_hook->follow_with(get_atom);
			get_atom->must_complete_before(let);
			get_atom->follow_with(_submit_result);

		}

		_frac_tasks[i].final_hook = final_hook;
		_frac_tasks[i].let_go = let;

		_first_tasks.push_back(first_hook);
	}
}

void SubmissionHelp::bundleWorkIfApplicable(int idx)
{
	if (!(_options & VdWClashes))
	{
		return;
	}

	TaskInfo &info = _frac_tasks[idx];
	float frac = idx / (float)_steps;

	BundleBonds *bbs = new BundleBonds(sequences()->sequence(), frac);

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
	                                + std::to_string(idx));

	info.bundle_hook = 
	new Task<BundleBonds *, BundleBonds *>(bundle_hook, "bundle hook "
	                                       + std::to_string(idx));

	for (int j = -1; j <= 2; j++)
	{
		int n = idx + j;
		if (n < 0 || n >= _steps)
		{
			continue;
		}

		TaskInfo &local_info = _frac_tasks[n];
		auto bundle_seq = [j, bbs](BondSequence *seq) -> BundleBonds *
		{
			*bbs += {j + 1, seq};
			return bbs;
		};

		_nBundles++;
		auto *bundle = new Task<BondSequence *, BundleBonds *>
		(bundle_seq, "bundle sequence " + std::to_string(_nBundles));

		local_info.final_hook->follow_with(bundle);
		bundle->follow_with(info.bundle_hook);
		bundle->must_complete_before(local_info.let_go);
	}

	_submit_result->must_complete_before(info.delete_bundle);
	_activation_energy_ref_submit->must_complete_before(info.delete_bundle);
	info.bundle_hook->follow_with(info.delete_bundle);
}

void SubmissionHelp::pairwiseWork(int idx)
{
	TaskInfo &info = _frac_tasks[idx];
	float frac = idx / (float)_steps;

	// act of setting up a bundle job, activation energy is relative to
	// energy of i = 0 and therefore gets an extra job.
	auto setup_submit_hooks =
	[this] <typename SpecificTask> (SpecificTask *task, int i)
	{
		task->follow_with(_submit_result);
		if (i == 0)
		{
			task->follow_with(_activation_energy_ref_submit);
		}
	};

	PairwiseDeviations *chosen = chosenCache();

	std::set<ResidueId> active_ids = 
	doingSides() ? _ids : std::set<ResidueId>();

	// deciding what to calculate from the bond sequences

	// if normal momentum
	if (!(_options & VdWClashes) && !(_options & ContactMap))
	{
		if (_options & PerResidue) // separated: one residue, one score
		{
			std::set<ResidueId> residues = chosen->residues();

			for (const ResidueId &id : residues)
			{
				if (_ids.size() > 0 && _ids.count(id) == 0)
				{
					continue;
				}
				Task<BondSequence *, Deviation> *task = nullptr;
				task = chosen->momentum_task(frac, {id});
				info.final_hook->follow_with(task);
				task->must_complete_before(info.let_go);
				setup_submit_hooks(task, idx);

				auto convert = [id, this](const Deviation &ae) 
				-> SingleResult<ResidueId>
				{
					return {id, ae.value / _steps};
				};

				auto *momentum_conv =
				new Task<Deviation, SingleResult<ResidueId>>
				(convert, "convert score to single residue result");

				task->follow_with(momentum_conv);
				momentum_conv->follow_with(submitResidueResult);
			}
		}
		else // all residues in one job, fullblown score
		{
			Task<BondSequence *, Deviation> *task = nullptr;
			task = chosen->momentum_task(frac, active_ids);
			info.final_hook->follow_with(task);
			task->must_complete_before(info.let_go);
			setup_submit_hooks(task, idx);
		}
	}
	else if (!(_options & ContactMap))
	{
		Task<BundleBonds *, ActivationEnergy> *task = nullptr;
		task = chosen->bundle_clash(active_ids);
		info.bundle_hook->follow_with(task);
		setup_submit_hooks(task, idx);
	}
	else // we want energies for a contact map
	{
		Task<BundleBonds *, Contacts> *task = nullptr;
		task = chosen->contact_map(active_ids);
		info.bundle_hook->follow_with(task);
		setup_submit_hooks(task, idx);
	}

	if (_options & PerResidue && _options & VdWClashes)
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
			if ((_options & TorsionEnergies) && !(_options & NoHydrogens))
			{
				EnergyTorsions *tEnergies = chosenEnergyTorsions();
				engy = tEnergies->energy_task({id}, frac);
				info.final_hook->follow_with(engy);
				engy->must_complete_before(info.let_go);
			}

			auto convert = [id, this](const ActivationEnergy &ae) 
			-> SingleResult<ResidueId>
			{
				return {id, ae.value / _steps};
			};

			auto *vdw_conv =
			new Task<ActivationEnergy, SingleResult<ResidueId>>(convert,
			                                                    "convert vdw to single residue result");

			task->follow_with(vdw_conv);
			vdw_conv->follow_with(submitResidueResult);

			if (engy)
			{
				auto *engy_conv =
				new Task<ActivationEnergy, 
				SingleResult<ResidueId>>(convert,
				                         "convert energy to single");

				engy->follow_with(engy_conv);
				engy_conv->follow_with(submitResidueResult);
			}
		}
	}

	// torsion energies
	if (_options & TorsionEnergies && !(_options & NoHydrogens) 
	    && !(_options & PerResidue))
	{
		EnergyTorsions *tEnergies = chosenEnergyTorsions();
		Task<BondSequence *, ActivationEnergy> *task = nullptr;
		task = tEnergies->energy_task(active_ids, frac);
		info.final_hook->follow_with(task);
		task->must_complete_before(info.let_go);
		setup_submit_hooks(task, idx);
	}
}

bool SubmissionHelp::doingSides()
{
	return (_options & WithSideChains);
}

void SubmissionHelp::pairwiseWorkIfApplicable()
{
	if (!(_options & Pairwise))
	{
		return;
	}
	
	for (int i = 0; i < _steps; i++)
	{
		bundleWorkIfApplicable(i);
		pairwiseWork(i);
	}
}

void SubmissionHelp::finaliseJobSubmission()
{
	_route->_ticket2Point[0] = 0;
	_route->_point2Score[0] = Route::Score{};

	_route->_ticket2Point[1] = 1;
	_route->_point2Score[1] = Route::Score{};
	
	for (BaseTask *t : _first_tasks)
	{
		_route->_resources.tasks->addTask(t);
	}

	BondCalculator *const &calculator = _route->_resources.calculator;
	calculator->releaseHorses();
	// perResBin needs to be released after collecting calculator results
}
