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

#ifndef __vagabond__SubmissionHelp__
#define __vagabond__SubmissionHelp__

#include <iostream>
#include "engine/ElementTypes.h"
#include "engine/Task.h"
#include "paths/BundleBonds.h"

class BondSequenceHandler;
class PairwiseDeviations;
class BondSequence;
class BaseTask;
class Route;

typedef Task<BondSequence *, BondSequence *> CalcTask;

enum CalcOptions
{
	None = 0,
	Pairwise = 1 << 0,
	NoHydrogens = 1 << 1,
	TorsionEnergies = 1 << 2,
	VdWClashes = 1 << 3,
	PerResidue = 1 << 4,
	ContactMap = 1 << 5,
};

inline std::ostream &operator<<(std::ostream &ss, const CalcOptions &opts)
{
	if (opts & Pairwise) ss << "Pairwise ";
	if (opts & NoHydrogens) ss << "NoHydrogens ";
	if (opts & TorsionEnergies) ss << "TorsionEnergies ";
	if (opts & VdWClashes) ss << "VdWClashes ";
	if (opts & PerResidue) ss << "PerResidue ";
	return ss;
}

struct TaskInfo
{
	CalcTask *final_hook;
	Task<BondSequence *, void *> *let_go;
	Task<BundleBonds *, void *> *delete_bundle;
	Task<BundleBonds *, BundleBonds *> *bundle_hook;
};

class SubmissionHelp
{
public:
	SubmissionHelp(Route *route, const CalcOptions &options);

	BondSequenceHandler *sequences();
	void prepareBinForScoreResult();
	void torsionPositionCalculation();

	void pairwiseWorkIfApplicable();
private:
	int _steps = 12;
	Route *_route = nullptr;
	CalcOptions _options{};

	void bundleWorkIfApplicable(int idx);
	void calculationExtractionFlags(Flag::Calc *calc,
	                                Flag::Extract *extract);
	void applyPostCalcTasks(CalcTask *&hook, const float &frac);

	PairwiseDeviations *chosenCache();
	std::vector<std::function<BondSequence *(BondSequence *)>> 
	extraTasks(const float &frac);
	
	BondSequenceHandler *_sequences{};
	
	// the submission task from the bin
	Task<Result, void *> *_submit_result{};
	Task<Result, void *> *_activation_energy_ref_submit{};

	// written to in torsion position calculation
	std::vector<BaseTask *> _first_tasks;
	std::map<int, TaskInfo> _frac_tasks;
	int _nBundles = 0;
};

#endif
