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

#include "Route.h"
#include "Polymer.h"
#include "Grapher.h"
#include "ParamSet.h"
#include "TorsionBasis.h"
#include "TorsionData.h"
#include "BondSequence.h"

#include "engine/Tasks.h"
#include "BondSequenceHandler.h"

Route::Route(Instance *from, Instance *to, const RTAngles &list)
{
	setInstance(from);
	_endInstance = to;
	_source = list;
	instance()->load();
}

Route::~Route()
{
	instance()->unload();
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

	submitJob(frac, show, job_num);
	retrieve();
	
	float ret = _point2Score.begin()->second.deviations;

	return ret;
}

void Route::submitJob(float frac, bool show, int job_num)
{
	_ticket++;
	Flag::Extract gets = Flag::Deviation;
	if (show)
	{
		gets = (Flag::Extract)(Flag::AtomVector | gets);
	}

	submitSingleAxisJob(frac, _ticket, gets);
	_ticket2Point[_ticket] = job_num;
	_point2Score[job_num] = Score{};
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

	BondCalculator *calc = _resources.calculator;
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

	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	CoordManager *manager = _resources.sequences->manager();
	manager->setAtomFetcher(AtomBlock::prepareMovingTargets(blocks));
}
