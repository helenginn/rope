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
		float frac = i / (float)_steps;

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

void SubmissionHelp::pairwiseWorkIfApplicable()
{
	if (!(_options & Pairwise))
	{
		return;
	}
	
	PairwiseDeviations *chosen = chosenCache();

	for (int i = 0; i < _steps; i++)
	{
		bundleWorkIfApplicable(i);
		

	}
}
