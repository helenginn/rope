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
#include "BondCalculator.h"
#include "BondSequence.h"
#include "Polymer.h"
#include "TorsionData.h"
#include "BondSequenceHandler.h"
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
	prepareTorsionFetcher(_resources.sequences);
	setTargets();

	if (_isNew)	
	{
		bestGuessTorsions();
	}

	prepareJobs();
}

auto supply_range = [](PlausibleRoute *me, float min, float max, bool mains_only)
{
	return [me, min, max, mains_only](int idx)
	{
		return me->validateTorsion(idx, min, max, mains_only, !mains_only);
	};
};

template <class Validate>
auto nudge_chain(PlausibleRoute *me, const Validate &validate,
                 const std::string &message, bool randomise)
{
	std::vector<size_t> indices; indices.reserve(me->motionCount());
	
	for (size_t i = 0; i < me->motionCount(); i++)
	{
		indices.push_back(i);
	}
	
	if (randomise)
	{
		std::random_shuffle(indices.begin(), indices.end());
	}

	return [me, validate, message, indices]()
	{
		me->nudgeTorsions(validate, message, indices);
	};
};

auto get_nudge_jobs(PlausibleRoute *me, bool mains)
{
	std::vector<PlausibleRoute::Task> nudge_cycle;

	me->setMaxMagnitude(360);
	auto range = supply_range(me, 0, 360, mains);
	auto task = nudge_chain(me, range, "Nudging angles", true);
	nudge_cycle.push_back(task);
	
	return nudge_cycle;
};

bool PlausibleRoute::meaningfulUpdate(float new_score, float old_score,
                                      float threshold)
{
	if (new_score < 0) return false;
	
	if (doingClashes() && new_score < 0.01)
	{
		return false;
	}

	bool good_ratio = (new_score < threshold * old_score);
	float required_reduction = doingClashes() ? 0.5 : 0.005;
	bool bad_reduction = (old_score - new_score < required_reduction);

	return good_ratio && !bad_reduction;
}

auto cycle_and_check(PlausibleRoute *me)
{
	return [me]()
	{
		int standard = me->nudgeCount();
		float oldsc = me->routeScore(standard);
		bool mains = !me->doingSides();

		std::vector<PlausibleRoute::Task> tasks = get_nudge_jobs(me, mains);

		for (PlausibleRoute::Task &task : tasks)
		{
			if (!me->shouldFinish())
			{
				task();
				me->finishTicker();
			}
		}

		me->clearIds();
		float newsc = me->routeScore(standard);
		me->postScore(newsc);
		
		if (me->doingClashes())
		{
			std::cout << oldsc << " to " << newsc << std::endl;
		}

		if (!me->shouldFinish() && !me->meaningfulUpdate(newsc, oldsc, 0.9))
		{
			me->upgradeJobs();
			if (me->lastJob())
			{
				me->finish();
			}
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
	
//	if (_maxFlipTrial > 0)
	{
		_tasks.push_back(flip_side_chain);
	}
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
	
	updateAtomFetch(_resources.sequences);
	updateAtomFetch(_mainChainSequences);
	updateAtomFetch(_hydrogenFreeSequences);
	preparePairwiseDeviations();
	prepareTorsionFetcher(_mainChainSequences);
	prepareTorsionFetcher(_hydrogenFreeSequences);
}

float PlausibleRoute::routeScore(int steps, bool pairwise)
{
	clearTickets();
	_resources.calculator->holdHorses();
	CalcOptions options = pairwise ? Pairwise : None;
	if (!doingClashes() && !doingSides())
	{
		options = (CalcOptions)(options | CoreChain);
	}
	if (!doingHydrogens())
	{
		options = (CalcOptions)(options | NoHydrogens);
	}

	for (size_t i = 0; i < steps; i++)
	{
		float show = (rand() / (double)RAND_MAX < 0.01);
		float frac = i / (float)steps;
		
		if (!_updateAtoms) show = false; // don't show

		submitJob(frac, show, options);
	}
	
	_resources.calculator->releaseHorses();
	retrieve();

	float sc = _point2Score[0].deviations;
	
	if (doingClashes())
	{
		_activationEnergy = _point2Score[0].highest_energy;
		_activationEnergy -= _point2Score[0].lowest_energy;
	}

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
	
	if (p && p->isConstrained())
	{
		return false;
	}

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

void PlausibleRoute::prepareAnglesForRefinement(const std::vector<int> &idxs)
{
	if (_simplex)
	{
		delete _simplex;
		_simplex = nullptr;
	}
	
	_simplex = new SimplexEngine(this);

	_paramPtrs.clear();
	_paramStarts.clear();
	_ids.clear();
	std::vector<float> steps;
	
	for (size_t i = 0; i < idxs.size(); i++)
	{
		if (idxs[i] < 0)
		{
			continue;
		}
		
		_ids.insert(parameter(idxs[i])->residueId());
		WayPoints &wps = wayPoints(idxs[i]);
		Motion &motion = _motions.storage(idxs[i]);
		
		float movement = fabs(destination(idxs[i]));
		float step = doingClashes() ? 5 : 1;

		_paramStarts.push_back(wps._grads[0]);
		_paramPtrs.push_back(&wps._grads[0]);
		steps.push_back(step);

		if (doingCubic() && motion.twist.twist)
		{
			_paramStarts.push_back(motion.twist.twist->twist);
			_paramPtrs.push_back(&motion.twist.twist->twist);
			steps.push_back(45.f);
		}

		if (doingCubic())
		{
			_paramStarts.push_back(wps._grads[1]);
			_paramPtrs.push_back(&wps._grads[1]);
			steps.push_back(step);
		}
	}
	
	_bestScore = routeScore(nudgeCount());
	
	int runs = doingCubic() ? 20 : 10;
	
	_simplex->setMaxRuns(runs);
	_simplex->chooseStepSizes(steps);
}

size_t PlausibleRoute::parameterCount()
{
	return _paramPtrs.size();
}

bool PlausibleRoute::simplexCycle(const std::vector<int> &torsionIdxs)
{
	prepareAnglesForRefinement(torsionIdxs);

	if (_paramPtrs.size() == 0)
	{
		return false;
	}

	_bestScore = routeScore(nudgeCount());
	
	if (doingClashes() && _bestScore < 0)
	{
		return false;
	}
	
	if (_bestScore <= 0)
	{
		return false;
	}
	
	_simplex->start();

	bool changed = false;
	float bs = routeScore(nudgeCount());

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
                                  const std::string &message,
                                  const std::vector<size_t> &indices)
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

	_ids.clear();

	for (size_t j = 0; j < indices.size(); j++)
	{
		if (Route::_finish)
		{
			return false;
		}

		clickTicker();

		size_t i = indices[j];
		Parameter *centre = parameter(i);

		if (!validate(i) || 
		    (!doingClashes() && motion(i).locked > 0) ||
		    (doingClashes() && motion(i).locked > 1))
		{
			continue;
		}
		
		OpSet<int> single;
		single.insert(indexOfParameter(centre));
		if (doingClashes() && false)
		{
//			ParamSet related = centre->relatedParameters();
//			single = convert_to_indices(related);
		}

		single.filter(validate);
		/*
		for (const int &idx : single)
		{
			std::cout << idx << "->" << parameter(idx) << ", ";
		}
		std::cout << std::endl;
		*/
		
		if (single.size() == 0)
		{
			continue;
		}
		
		bool result = simplexCycle(single.toVector());
		
		if (!result && single.size() == 1)
		{
			int &lock = motion(*single.begin()).locked;
			lock++;
		}
		/*
		else if (result && single.size() == 1)
		{
			int &lock = motion(*single.begin()).locked;
			if (lock > 0)
			{
				std::cout << "Editing locked param " << lock << std::endl;
			}
		}
		*/
		
		changed += (result ? 1 : 0);
	}

	_ids.clear();
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
	const int max = _maxFlipTrial;
	
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
	
	bool pairwise = doingClashes();

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

		float candidate = routeScore(flipNudgeCount(), pairwise);

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

	bool pairwise = doingClashes();
	if (pairwise) return false;

	_bestScore = routeScore(flipNudgeCount(), pairwise);
	float min = doingClashes() ? 10 : 90;
	
	for (size_t i = 0; i < motionCount(); i++)
	{
		if (Route::_finish)
		{
			return false;
		}

		clickTicker();
		
		if (!validate(i) || fabs(destination(i)) < min)
		{
			continue;
		}

		bool changed_last = flipTorsion(validate, i);
		changed |= changed_last;
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

	_bestScore = routeScore(flipNudgeCount(), true);
	postScore(_bestScore);
}


void PlausibleRoute::flipTorsionCycles(const ValidateParam &validate)
{
	flipTorsionCycle(validate);
}

void PlausibleRoute::cycle()
{
	while (!lastJob())
	{
		for (PlausibleRoute::Task &task : _tasks)
		{
			task();
			finishTicker();
			
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
	std::cout << "Sending response now" << std::endl;
	Route::sendResponse("done", (void *)this);
}

void PlausibleRoute::prepareTorsionFetcher(BondSequenceHandler *handler)
{
	TorsionBasis *basis = handler->sequence()->torsionBasis();
	const std::vector<Parameter *> &params = basis->parameters();

	std::vector<int> lookup;
	lookup.resize(params.size());
	
	for (int i = 0; i < params.size(); i++)
	{
		Parameter *bondseq_param = params[i];
		int motion_idx = indexOfParameter(bondseq_param);
		if (bondseq_param->isConstrained())
		{
			motion_idx = -1;
		}
		lookup[i] = motion_idx;
	}

	auto fetch = [this, lookup](const Coord::Get &get, int torsion_idx)
	{
		int mot_idx = lookup[torsion_idx];
		if (mot_idx < 0) return 0.f;

		float t = motion(mot_idx).interpolatedAngle(get(0));
		if (t != t) t = 0;
		return t;
	};

	handler->manager()->setTorsionFetcher(fetch);
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
	float result = routeScore(nudgeCount());
	postScore(result);
	int steps = 200;
	
	for (size_t i = 0; i < steps; i++)
	{
		float frac = i / (float)steps;
		submitJob(frac, true);
	}
	
	retrieve();
	clearTickets();
}

void PlausibleRoute::upgradeJobs()
{
	if (lastJob()) { return; }

	_jobLevel++;
	
	unlockAll();
	std::cout << "Job level now " << _jobLevel << "..." << std::endl;
}


void PlausibleRoute::refreshScores()
{
	_jobLevel = 0;
	_momentum = routeScore(nudgeCount());

	_jobLevel = 4;
	_clash = routeScore(nudgeCount());
	std::cout << "Minimise momentum score " << _momentum << std::endl;
	std::cout << "Minimise clash score " << _clash << std::endl;
	std::cout << "Activation energy " << _activationEnergy << std::endl;
	
	_gotScores = true;
}
