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

#include "ParamSet.h"
#include "PlausibleRoute.h"
#include "Polymer.h"
#include "MetadataGroup.h"
#include <vagabond/utils/maths.h>
#include "Grapher.h"
#include <vagabond/c4x/Cluster.h>

PlausibleRoute::PlausibleRoute(Instance *from, Instance *to, const RTAngles &list)
: Route(from, to, list)
{
	_threads = 1;
}

void PlausibleRoute::setup()
{
	Route::setup();
	prepareDestination();
	prepareTorsionFetcher();
	setTargets();
	prepareJobs();
}

auto supply_range = [](PlausibleRoute *me, float min, float max, bool mains_only)
{
	return [me, min, max, mains_only](int idx)
	{
		return me->validateTorsion(idx, min, max, mains_only, false);
	};
};

template <class Validate>
auto nudge_chain(PlausibleRoute *me, const Validate &validate,
                 const std::string &message)
{
	return [me, validate, message]()
	{
		me->nudgeTorsions(validate, message);
	};
};

auto get_nudge_jobs(PlausibleRoute *me, bool mains)
{
	std::vector<PlausibleRoute::Task> nudge_cycle;
	std::vector<float> maxes = {1, 2, 5, 10, 20, 45, 90, 180, 360};

	for (float &max : maxes)
	{
		float min = max / 4;
		if (min < 0.5) min = 0.;
		std::string str = "Nudging angles from " + f_to_str(min, 1) + " to "
		+ f_to_str(max, 1) + " degrees";
		auto task = nudge_chain(me, supply_range(me, min, max, mains), str);
		nudge_cycle.push_back(task);
		                                                 
	}
	
	return nudge_cycle;
};

auto cycle_and_check(PlausibleRoute *me)
{
	return [me]()
	{
		int standard = 12;
		float oldsc = me->routeScore(standard);
		bool mains = (me->jobLevel() <= 1);

		std::vector<PlausibleRoute::Task> tasks = get_nudge_jobs(me, mains);

		for (PlausibleRoute::Task &task : tasks)
		{
			task();
		}

		float newsc = me->routeScore(standard);

		if (fabs(newsc - oldsc) < 0.002 && me->jobLevel() > 1)
		{
			me->finish();
		}
		else if (newsc > 0.9 * oldsc)
		{
			me->upgradeJobs();
		}
	};
};

struct only_once
{
	only_once(const PlausibleRoute::Task &func) : _func(func) {};

	void operator()()
	{
		if (!_done)
		{
			_func();
			_done = true;
		}
	}

	const PlausibleRoute::Task _func;
	bool _done = false;
};

void PlausibleRoute::prepareJobs()
{
	auto large_main = [this](int idx) -> bool
	{
		return validateTorsion(idx, 30, 360., true, false);
	};

	auto large_sides = [this](int idx) -> bool
	{
		return validateTorsion(idx, 30, 360., false, true);
	};

	auto flip_main_chain = [this, large_main]()
	{
		flipTorsionCycles(large_main);
	};

	auto flip_side_chain = [this, large_sides]()
	{
		flipTorsionCycles(large_sides);
	};

	only_once flip_once(flip_main_chain);
	
	_tasks.push_back(flip_once);
	_tasks.push_back(cycle_and_check(this));
	_tasks.push_back(flip_side_chain);
}

void PlausibleRoute::setTargets()
{
	std::map<Atom *, glm::vec3> atomStart;
	AtomGroup *grp = instance()->currentAtoms();

	submitJobAndRetrieve(0., true);

	for (Atom *atom : grp->atomVector())
	{
		glm::vec3 d = atom->derivedPosition();
		atomStart[atom] = d;
	}

	submitJobAndRetrieve(1., true);

	for (Atom *atom : grp->atomVector())
	{
		glm::vec3 d = atom->derivedPosition();
		glm::vec3 s = atomStart[atom];
		glm::vec3 diff = (d - s);
		atom->setOtherPosition("moving", diff);
		atom->setOtherPosition("target", s);
	}
}

float PlausibleRoute::routeScore(int steps, bool forceField)
{
	clearTickets();

	for (size_t i = 0; i < steps; i++)
	{
		float show = (rand() / (double)RAND_MAX < 0.01);
		float frac = i / (float)steps;
		
		if (!_updateAtoms) show = false; // don't show

		submitJob(frac, show, 0);
	}
	
	retrieve();

	float sc = _point2Score[0].deviations;
	clearTickets();
	
	return sc;
}

void PlausibleRoute::startTicker(std::string tag, int d)
{
	if (d < 0)
	{
		d = motionCount();
	}

	HasResponder<Responder<Route>>::sendResponse("progress_" + tag, 
	                                             &d);
}

void PlausibleRoute::postScore(float score)
{
	HasResponder<Responder<Route>>::sendResponse("score", &score);
}

bool PlausibleRoute::validateTorsion(int idx, float min_mag, 
                                     float max_mag, bool mains_only, 
                                     bool sides_only)
{
	if (idx < 0)
	{
		return false;
	}

	Parameter *const &p = parameter(idx);

	if (!p || (mains_only && !p->coversMainChain())
	    || (sides_only && p->coversMainChain()))
	{
		return false;
	}

	float magnitude = fabs(destination(idx));
	if ((min_mag >= 0 && magnitude < min_mag) ||
	     (max_mag >= 0 && magnitude > max_mag))
	{
		return false;
	}

	return true;
}

void PlausibleRoute::prepareAnglesForRefinement(std::vector<int> &idxs)
{
	if (_simplex)
	{
		delete _simplex;
		_simplex = nullptr;
	}
	
	_simplex = new SimplexEngine(this);

	_paramPtrs.clear();
	_paramStarts.clear();
	std::vector<float> steps;
	
	for (size_t i = 0; i < idxs.size(); i++)
	{
		if (idxs[i] < 0)
		{
			continue;
		}

		WayPoints &wps = wayPoints(idxs[i]);

		_paramStarts.push_back(wps._grads[0]);
		_paramPtrs.push_back(&wps._grads[0]);
		steps.push_back(0.1);

		if (_jobLevel > 0)
		{
			_paramStarts.push_back(wps._grads[1]);
			_paramPtrs.push_back(&wps._grads[1]);
			steps.push_back(0.1);
		}
	}
	
	_simplex->setMaxRuns(20);
	_simplex->chooseStepSizes(steps);
}

size_t PlausibleRoute::parameterCount()
{
	return _paramPtrs.size();
}

bool PlausibleRoute::simplexCycle(std::vector<int> torsionIdxs)
{
	prepareAnglesForRefinement(torsionIdxs);

	if (_paramPtrs.size() == 0)
	{
		return false;
	}

	_bestScore = routeScore(_nudgeCount);

	_simplex->start();

	bool changed = false;
	float bs = routeScore(_nudgeCount);

	if (bs < _bestScore - 1e-3)
	{
		_bestScore = bs;
		postScore(bs);
		changed = true;
	}
	else
	{
		std::vector<float> empty(_paramPtrs.size(), 0);
		assignParameterValues(empty);
	}

	return changed;
}

int PlausibleRoute::nudgeTorsions(const ValidateParam &validate,
                                  const std::string &message)
{
	int changed = 0;
	startTicker(message);

	auto convert_to_indices = [this](ParamSet &set) -> OpSet<int>
	{
		OpSet<int> indices = set.convert_to<int>([this](Parameter *p)
		                                         {
			                                        return indexOfParameter(p);
		                                         });
		return indices;
	};

	float start = routeScore(flipNudgeCount());

	for (size_t i = 0; i < motionCount(); i++)
	{
		if (Route::_finish)
		{
			return false;
		}

		clickTicker();

		Parameter *centre = parameter(i);

		if (!validate(i))
		{
			continue;
		}
		
		ParamSet related = centre->relatedParameters();
		OpSet<int> single;
//		single.push_back(indexOfParameter(centre));
		single = convert_to_indices(related);
		single.filter(validate);
		
		bool result = simplexCycle(single.toVector());
		
		changed += (result ? 1 : 0);
	}

	postScore(_bestScore);
	finishTicker();
	
	return changed;
}

void print(std::vector<int> &flips)
{
	std::cout << flips.size() << ": ";
	for (size_t j = 0; j < flips.size(); j++)
	{
		std::cout << (flips[j] ? "Y" : "n");
	}
}


std::vector<int> PlausibleRoute::getTorsionSequence(int idx,
                                                    const ValidateParam &validate)
{
	if (!validate(idx))
	{
		return std::vector<int>();
	}

	AtomGraph *g = grapherForTorsionIndex(idx);

	std::vector<int> idxs;
	if (!g) return idxs;

	int count = 0;
	idxs.push_back(idx);
	const int max = 5;
	
	while (g && count < max)
	{
		for (size_t j = 0; j < g->children.size(); j++)
		{
			Atom *a = g->children[j]->atom;
			AtomGraph *candidate = grapher().graph(a);
			int n = indexOfParameter(candidate->torsion);

			if ((n < 0) || !validate(n))
			{
				continue;
			}

			idxs.push_back(n);
			count++;
		}

		g = g->deepestChild();
	}

	return idxs;
}


bool PlausibleRoute::flipTorsion(const ValidateParam &validate, int idx)
{
	std::vector<int> idxs = getTorsionSequence(idx, validate);
	
	if (idxs.size() == 0)
	{
		return false;
	}

	std::vector<int> best(idxs.size(), false);
	for (size_t i = 0; i < idxs.size(); i++)
	{
		best[i] = flip(idxs[i]);
	}

	std::vector<std::vector<int> > putatives = permutations(idxs.size());
	bool changed = false;
	for (size_t i = 0; i < putatives.size(); i++)
	{
		setFlips(idxs, putatives[i]);

		float candidate = routeScore(flipNudgeCount());

		if (candidate < _bestScore - 1e-3)
		{
			_bestScore = candidate;
			best = putatives[i];
			changed = true;
		}
	}

	setFlips(idxs, best);
	postScore(_bestScore);
	
	return changed;
}

bool PlausibleRoute::flipTorsions(const ValidateParam &validate)
{
	startTicker("Flipping torsions");
	bool changed = false;

	_bestScore = routeScore(flipNudgeCount());
	
	for (size_t i = 0; i < motionCount(); i++)
	{
		if (Route::_finish)
		{
			return false;
		}

		clickTicker();
		
		if (!validate(i) || fabs(destination(i)) < 90)
		{
			continue;
		}

		changed |= flipTorsion(validate, i);
	}
	
	finishTicker();

	return changed;
}

void PlausibleRoute::flipTorsionCycle(const ValidateParam &validate)
{
	int count = 0;

	while (flipTorsions(validate) && count < 100)
	{
		if (Route::_finish)
		{
			return;
		}
		
		count++;
	}
}


void PlausibleRoute::flipTorsionCycles(const ValidateParam &validate)
{
	flipTorsionCycle(validate);
}

void PlausibleRoute::cycle()
{
	while (_jobLevel < 3)
	{
		for (PlausibleRoute::Task &task : _tasks)
		{
			task();
			if (Route::_finish)
			{
				return;
			}
		}
	}

	finishTicker();
}

void PlausibleRoute::doCalculations()
{
	if (!Route::_finish)
	{
		cycle();
	}
	
	finishTicker();
	prepareForAnalysis();
	Route::_finish = false;
}

void PlausibleRoute::prepareTorsionFetcher()
{
	auto fetch = [this](const Coord::Get &get, int torsion_idx)
	{
		float t = motion(torsion_idx).interpolatedAngle(get(0));
		if (t != t) t = 0;
		return t;
	};

	_fetchTorsion = fetch;
}

void PlausibleRoute::assignParameterValues(const std::vector<float> &trial)
{
	assert(trial.size() == _paramPtrs.size());

	for (size_t i = 0; i < _paramPtrs.size(); i++)
	{
		float value = _paramStarts[i] + trial[i];
		*(_paramPtrs[i]) = value;
	}
}

int PlausibleRoute::sendJob(const std::vector<float> &all)
{
	assignParameterValues(all);
	float result = FLT_MAX;
	
	int num = std::min((int)_magnitudeThreshold * 2, 32);
	num = std::max(num, 4);
	result = routeScore(num);
	
	int ticket = getNextTicket();
	setScoreForTicket(ticket, result);
	return ticket;
}

void PlausibleRoute::prepareForAnalysis()
{
	float result = routeScore(_nudgeCount);
	postScore(result);
	int steps = 200;
	
	for (size_t i = 0; i < steps; i++)
	{
		float frac = i / (float)steps;
		submitJob(frac, true, true);
	}
	
	retrieve();
	clearTickets();
}

void PlausibleRoute::upgradeJobs()
{
	if (_jobLevel > 3) { return; }

	std::cout << "Splitting..." << std::endl;
	_jobLevel++;
	
	if (_jobLevel == 2)
	{
		_mainsOnly = false;
	}
}

