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

#include "Route.h"
#include "Polymer.h"
#include "Grapher.h"
#include "ParamSet.h"
#include "TorsionBasis.h"
#include "TorsionData.h"
#include "BondSequence.h"
#include "BondCalculator.h"
#include "PairwiseDeviations.h"
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
	instance()->load();
}

Route::~Route()
{
	instance()->unload();
	delete _pwMain;
	delete _pwHeavy;
	delete _pwEvery;
}

void Route::setup()
{
	if (_source.size() == 0 && motionCount() == 0)
	{
		throw std::runtime_error("No destination or prior set for route");
	}

	prepareResources();
}

float Route::submitJobAndRetrieve(float frac, bool show)
{
	clearTickets();

	_resources.calculator->holdHorses();

	submitJob(frac, show);

	_resources.calculator->releaseHorses();
	retrieve();
	
	float ret = _point2Score.begin()->second.deviations;

	return ret;
}

void Route::submitJob(float frac, bool show, const CalcOptions &options)
{
	_ticket++;
	bool pairwise = (options & Pairwise);
	bool coreChain = (options & CoreChain);
	bool hydrogens = !(options & NoHydrogens);

	Flag::Extract gets = !pairwise ? Flag::Deviation : Flag::NoExtract;
	if (show)
	{
		gets = (Flag::Extract)(Flag::AtomVector | gets);
	}

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *sequences = 
	coreChain ? _mainChainSequences : 
	(hydrogens ? _resources.sequences : _hydrogenFreeSequences);

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(0);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions);
	if (show || !pairwise)
	{
		calc = Flag::Calc(Flag::DoSuperpose | calc);
	}

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, {frac}, &first_hook, &final_hook);

	Task<BondSequence *, void *> *let = 
	sequences->extract(gets, submit_result, final_hook);
	
	if (pairwise)
	{
		std::set<ResidueId> empty;
		if (!doingClashes())
		{
			PairwiseDeviations *chosen = doingSides() ? _pwHeavy : _pwMain;
			Task<BondSequence *, Deviation> *task = nullptr;

			task = chosen->momentum_task(doingSides() ? _ids : empty);
			final_hook->follow_with(task);
			task->must_complete_before(let);
			task->follow_with(submit_result);
		}
		else
		{
			PairwiseDeviations *chosen = hydrogens ? _pwEvery : _pwHeavy;
			Task<BondSequence *, ActivationEnergy> *task = nullptr;
			task = chosen->clash_task(doingSides() ? _ids : empty);
			final_hook->follow_with(task);
			task->must_complete_before(let);
			task->follow_with(submit_result);
		}

	}

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

float best_guess_torsion(Parameter *param)
{
	if (!param->isTorsion())
	{
		return 0;
	}

	glm::vec3 before[4]{};
	glm::vec3 after[4]{};

	for (size_t i = 0; i < param->atomCount(); i++)
	{
		before[i] = param->atom(i)->otherPosition("target");
		after[i] = param->atom(i)->otherPosition("moving");
	}

	float first = 0;
	float last = 0;
	for (float f = 0; f <= 1.01; f += 0.1)
	{
		glm::vec3 frac[4]{};
		for (int i = 0; i < 4; i++)
		{
			frac[i] = before[i] + after[i] * f;
		}
		
		float torsion = measure_bond_torsion(frac);
		if (f <= 0)
		{
			first = torsion;
			last = torsion;
		}
		else
		{
			while (torsion < last - 180.f) torsion += 360.f;
			while (torsion >= last + 180.f) torsion -= 360.f;
		}
		

		last = torsion;
	}

	return last - first;
}

void Route::bestGuessTorsion(int idx)
{
	if (!parameter(idx)->isTorsion())
	{
		return;
	}

	destination(idx) = best_guess_torsion(parameter(idx));
}

void Route::bestGuessTorsions()
{
	for (size_t i = 0; i < motionCount(); i++)
	{
		if (fabs(destination(i)) > 30)
		{
			bestGuessTorsion(i);
		}
	}
}

void Route::bringTorsionsToRange()
{
	for (size_t i = 0; i < motionCount(); i++)
	{
		while (destination(i) >= 180)
		{
			destination(i) -= 360;
		}
		while (destination(i) < -180)
		{
			destination(i) += 360;
		}
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
}

void Route::prepareParameters()
{
	auto make_motion = [this](Parameter *const &param, ResidueTorsion &rt)
	{
		rt = ResidueTorsion(param);
		rt.attachToInstance(_instance);

		// index may be in existing motions, or in the source torsions
		int mot_idx = _motions.indexOfHeader(rt);
		int src_idx = _source.indexOfHeader(rt);
		int twst_idx = _twists.indexOfHeader(rt);

		Motion mt = {WayPoints(), false, 0};

		if (mot_idx >= 0)
		{
			Motion &target = _motions.storage(mot_idx);
			mt = target;
			if (mt.twist.twist && twst_idx >= 0)
			{
				_twists.storage(twst_idx).twist = mt.twist.twist->twist;
				_twists.storage(twst_idx).twist = mt.twist.twist->twist;

			}
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

		return mt;
	};

	getParametersFromBasis(make_motion);
	
}

void Route::setTwists(const RTPeptideTwist &twists)
{
	_twists = twists;
	_motions.incorporate(_twists);
}
	
void Route::prepareTwists()
{
	_twists = RTPeptideTwist::empty_twists(_motions.headers_only());
	_motions.incorporate(_twists);
	
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
	for (size_t i = 0; i < motionCount(); i++)
	{
		if (parameter(i) == t)
		{
			return i;
		}
	}
	
	return -1;
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
	_mainChainSequences = new BondSequenceHandler(_threads);
	_hydrogenFreeSequences = new BondSequenceHandler(_threads);

	AtomGroup *group = _instance->currentAtoms();

	std::vector<AtomGroup *> subsets = group->connectedGroups();
	for (AtomGroup *subset : subsets)
	{
		Atom *anchor = subset->chosenAnchor();
		_resources.sequences->addAnchorExtension(anchor);
		_mainChainSequences->addAnchorExtension(anchor);
		_hydrogenFreeSequences->addAnchorExtension(anchor);
	}

	_mainChainSequences->setIgnoreHydrogens(true);
	_mainChainSequences->setAtomFilter(rope::atom_is_core_main_chain());
	_mainChainSequences->setup();
	_mainChainSequences->prepareSequences();

	_hydrogenFreeSequences->setIgnoreHydrogens(true);
	_hydrogenFreeSequences->setAtomFilter(rope::atom_is_not_hydrogen());
	_hydrogenFreeSequences->setup();
	_hydrogenFreeSequences->prepareSequences();

	_resources.sequences->setup();
	_resources.sequences->prepareSequences();

	updateAtomFetch(_resources.sequences);
	updateAtomFetch(_mainChainSequences);
	updateAtomFetch(_hydrogenFreeSequences);
	preparePairwiseDeviations();
}

void Route::preparePairwiseDeviations()
{
	delete _pwMain;
	delete _pwHeavy;
	delete _pwEvery;

	auto main_chain_filter = [](Atom *const &atom)
	{
		return atom->atomName() == "CA" || atom->atomName() == "O"
			   || atom->atomName() == "C" || atom->atomName() == "N";
	};

	auto side_chain_filter = [](Atom *const &atom)
	{
		return (atom->elementSymbol() != "H");
	};

	const float limit = 8.f;
	_pwMain = new PairwiseDeviations(_mainChainSequences->sequence(),
									 {}, _maxMomentumDistance);

	_pwHeavy = new PairwiseDeviations(_hydrogenFreeSequences->sequence(),
									 {}, _maxClashDistance);

	_pwEvery = new PairwiseDeviations(_resources.sequences->sequence(),
									  {}, _maxClashDistance);
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
		motion(i).wp = {};
	}

	for (size_t i = 0; i < twistCount(); i++)
	{
		twist(i).twist = {};
	}
	
	_jobLevel = 0;
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

