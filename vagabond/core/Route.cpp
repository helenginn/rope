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

#include "Route.h"
#include "Polymer.h"
#include "Grapher.h"
#include "ParamSet.h"
#include "TorsionBasis.h"
#include "TorsionData.h"
#include "BondSequence.h"
#include "BondCalculator.h"
#include "PairwiseDeviations.h"

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
	delete _pwSide;
}

void Route::setup()
{
	if (_source.size() == 0 && motionCount() == 0)
	{
		throw std::runtime_error("No destination or prior set for route");
	}

	prepareResources();
}

float Route::submitJobAndRetrieve(float frac, bool show, int job_num)
{
	clearTickets();

	_resources.calculator->holdHorses();

	submitJob(frac, show, job_num);

	_resources.calculator->releaseHorses();
	retrieve();
	
	float ret = _point2Score.begin()->second.deviations;

	return ret;
}

void Route::submitJob(float frac, bool show, int job_num, bool pairwise)
{
	_ticket++;
	Flag::Extract gets = pairwise ? Flag::NoExtract : Flag::Deviation;
//	gets = Flag::NoExtract;
	if (show)
	{
		gets = (Flag::Extract)(Flag::AtomVector | gets);
	}

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(job_num);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions |
	                             Flag::DoSuperpose);

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, {frac}, &first_hook, &final_hook);

	Task<BondSequence *, void *> *let = 
	sequences->extract(gets, submit_result, final_hook);
	
	if (pairwise)
	{
		Task<BondSequence *, Deviation> *task = nullptr;
		if (!doingSides() || !pairwise)
		{
			task = _pwMain->normal_task(doingPreClash());
		}
		else
		{
			task = _pwSide->clash_task(_ids);
		}

		final_hook->follow_with(task);
		task->must_complete_before(let);
		task->follow_with(submit_result);
	}

	_ticket2Point[job_num] = job_num;
	_point2Score[job_num] = Score{};
	
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

void Route::bestGuessTorsion(int idx)
{
	if (!parameter(idx)->isTorsion())
	{
		return;
	}

	glm::vec3 before[4]{};
	glm::vec3 after[4]{};

	for (size_t i = 0; i < parameter(idx)->atomCount(); i++)
	{
		before[i] = parameter(idx)->atom(i)->otherPosition("target");
		after[i] = parameter(idx)->atom(i)->otherPosition("moving");
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
	
	destination(idx) = last - first;

//	std::cout << time(NULL) << std::endl;
	if (fabs(last - first) > 90.f)
	{
		int rnd = int(rand() % 5);
		if (//parameter(idx)->residueId() != 59 &&
		                parameter(idx)->residueId() != 61)
		{ 
			if (parameter(idx)->coversMainChain())
			{
				std::cout << "Skipping " << parameter(idx) << std::endl;
			}
			return;
		}

		int dir = (last - first > 0) ? -1 : 1;
		if (parameter(idx)->coversMainChain())
		{
			std::cout << "Flipping " << parameter(idx)->desc() << " ";
			std::cout << destination(idx) << " to ";
		}
		destination(idx) += dir * 360;
		if (parameter(idx)->coversMainChain())
		{
			std::cout << destination(idx) << std::endl;
		}
	}
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

void Route::getParametersFromBasis()
{
	if (_motions.size() > 0)
	{
		std::cout << "skipping pick up of parameters, done already" << std::endl;
		return;
	}

	ParamSet missing;

	std::vector<Motion> tmp_motions;
	std::vector<ResidueTorsion> torsions;

	TorsionBasis *basis = _resources.sequences->torsionBasis();

	for (size_t i = 0; i < basis->parameterCount(); i++)
	{
		Parameter *p = basis->parameter(i);
		int idx = _instance->indexForParameterFromList(p, _source);

		if (idx < 0)
		{
			torsions.push_back(ResidueTorsion{});
			tmp_motions.push_back(Motion{WayPoints(), false, 0});
			missing.insert(p);
		}
		else
		{
			ResidueTorsion rt = _source.c_rt(idx);
			rt.attachToInstance(_instance);
			float final_angle = _source.storage(idx);

			if (final_angle != final_angle) 
			{
				final_angle = 0;
			}

			torsions.push_back(rt);
			tmp_motions.push_back(Motion{WayPoints(), false, final_angle});
		}

	}

	_motions = RTMotion::motions_from(torsions, tmp_motions);
	bringTorsionsToRange();
	
	std::cout << "Missing: " << missing << std::endl;
}

void Route::prepareDestination()
{
	if (_source.size() == 0 && _motions.size() == 0)
	{
		throw std::runtime_error("Raw destination not set, nor motions for "\
		                         "destination");
	}
	
	getParametersFromBasis();
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

	AtomGroup *group = _instance->currentAtoms();

	std::vector<AtomGroup *> subsets = group->connectedGroups();
	for (AtomGroup *subset : subsets)
	{
		Atom *anchor = subset->chosenAnchor();
		_resources.sequences->addAnchorExtension(anchor);
	}

	_resources.sequences->setIgnoreHydrogens(true);
	_resources.sequences->setup();
	_resources.sequences->prepareSequences();

	updateAtomFetch();
}

void Route::updateAtomFetch()
{
	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	CoordManager *manager = _resources.sequences->manager();
	manager->setAtomFetcher(AtomBlock::prepareMovingTargets(blocks));
	
	delete _pwMain;
	delete _pwSide;

	auto main_chain_filter = [](Atom *const &atom)
	{
		return atom->atomName() == "CA" || atom->atomName() == "O";
		return (atom->atomName() == "N" || atom->atomName() == "C" ||
		        atom->atomName() == "O" || atom->atomName() == "CA");
	};

	auto side_chain_filter = [](Atom *const &atom)
	{
		return (atom->elementSymbol() != "H");
	};

	const float limit = 8.f;
	_pwMain = new PairwiseDeviations(_resources.sequences->sequence(),
	                                 main_chain_filter, limit);

	_pwSide = new PairwiseDeviations(_resources.sequences->sequence(),
	                                 side_chain_filter, 15.f);
}
