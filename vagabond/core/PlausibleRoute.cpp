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
#include "ResidueId.h"
#include "PlausibleRoute.h"
#include "BondCalculator.h"
#include "MultiSimplex.h"
#include "GradientTerm.h"
#include "BondSequence.h"
#include "Polymer.h"
#include "TorsionData.h"
#include "BondSequenceHandler.h"
#include <vagabond/utils/maths.h>
#include "Grapher.h"
#include "paths/ResidueContacts.h"
#include <vagabond/c4x/Cluster.h>

PlausibleRoute::PlausibleRoute(Instance *from, Instance *to, const RTAngles &list)
: Route(from, to, list)
{
	_threads = 1;
}

PlausibleRoute::PlausibleRoute(const RTAngles &list) : Route(list)
{

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
	
	if (false && randomise)
	{
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(indices.begin(), indices.end(), g);
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
	if (doingClashes() && old_score < 0 && new_score < 0)
	{
		return old_score - new_score > 10;
	}
	else if (old_score * new_score < 0)
	{
		return true;
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
		if (!me->doingClashes()) 
		{
			return;
		}
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

		if (!me->shouldFinish() && !me->meaningfulUpdate(newsc, oldsc, 0.99))
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
		return validateTorsion(idx, 120, 360., true, false);
	};

	/*
	auto large_sides = [this](int idx) -> bool
	{
		return validateTorsion(idx, 30, 360., false, true);
	};
	*/

	auto flip_main_chain = [this, large_main]()
	{
		flipTorsionCycles(large_main);
	};

	/*
	auto flip_side_chain = [this, large_sides]()
	{
		flipTorsionCycles(large_sides);
	};
	*/

	auto check_gradients = [this]
	(const std::function<bool()> &gradient_type)
	{
		return [this, gradient_type]()
		{
			bool good = true;
			float before = routeScore(nudgeCount(), None, CoreChain);
			while (true || !doingClashes())
			{
				good = gradient_type();
				if (!good)
				{
					if (!_finish)
					{
						upgradeJobs();
						if (lastJob())
						{
							finish();
						}
					}

					float after = _bestScore;
					std::cout << before << " to " << after << std::endl;
					break;
				}
			}
		};
	};

	only_once flip_once(flip_main_chain);
	_tasks.push_back(flip_once);
	_tasks.push_back(check_gradients([this]()
	                                 {
		                                if (!doingClashes())
										{
											return applyGradients({});
										}
										else
										{
											return perResidueGradients(); 
										}
									}));
//	_tasks.push_back(cycle_and_check(this));
}

void PlausibleRoute::setTargets(Instance *inst)
{
	std::map<Atom *, glm::vec3> atomStart;
	AtomGroup *grp = inst->currentAtoms();

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

void PlausibleRoute::setTargets()
{
	for (size_t i = 0; i < instanceCount(); i++)
	{
		setTargets(startInstance(i));
	}
	
	updateAtomFetch(_resources.sequences);
	updateAtomFetch(_mainChainSequences);
	updateAtomFetch(_hydrogenFreeSequences);
	prepareEnergyTerms();
	prepareTorsionFetcher(_mainChainSequences);
	prepareTorsionFetcher(_hydrogenFreeSequences);
}

PlausibleRoute::CalcOptions 
PlausibleRoute::calcOptions(const CalcOptions &add_options,
                                        const CalcOptions &subtract_options)
{
	CalcOptions options = Pairwise;

	if (!doingClashes() && !doingSides())
	{
		options = (CalcOptions)(options | CoreChain);
	}
	if (!doingHydrogens())
	{
		options = (CalcOptions)(options | NoHydrogens);
	}
	if (doingClashes())
	{
		options = (CalcOptions)(options | Pairwise | VdWClashes | TorsionEnergies);
	}
	if (doingSides())
	{
		options = (CalcOptions)(options | PerResidue);
	}
	
	options = Route::CalcOptions(options & (~subtract_options));
	options = Route::CalcOptions(options | (add_options));
	
	return options;
}

std::vector<Floats> save_current(GradientPath *path, const RTMotion &motions)
{
	auto current = std::vector<Floats>(motions.size());

	for (int i = 0; i < path->motion_idxs.size(); i++)
	{
		int p = path->motion_idxs[i];
		if (p >= 0 && p < motions.size())
		{
			current[p] = motions.storage(p).wp._amps;
		}
	}

	return current;
}

// you'll probably have to optimise this by working directly 
// from path->motion_idxs
std::map<int, int> indexed_motions(ByResidueResult *r, RTMotion &motions)
{
	std::map<int, int> indexed;
	int n = 0;
	for (auto it = r->scores.begin(); it != r->scores.end(); it++)
	{
		for (int i = 0; i < motions.size(); i++)
		{
			if (motions.rt(i).local_id() == it->first)
			{
				indexed[i] = n; // motion -> residue
			}
		}
		n++;
	}

	return indexed;
}

bool PlausibleRoute::perResidueGradients()
{
	_contacts = nullptr;

	_paramPtrs.clear();
	_paramStarts.clear();
	_steps.clear();

	float step = 10;
	std::map<ResidueId, std::vector<int>> map;
	int n = 0;
	for (int i = 0; i < motionCount(); i++)
	{
		ResidueTorsion &rt = residueTorsion(i);
		if (parameter(i)->coversMainChain() || parameter(i)->isConstrained())
		{
			continue;
		}

		const ResidueId &local = rt.local_id();
		WayPoints &wps = wayPoints(i);

		addFloatParameter(&wps._amps[0], step);
		addFloatParameter(&wps._amps[1], step);

		for (int j = 0; j < 2; j++)
		{
			map[local].push_back(n + j);
		}

		n += 2;
	}

	float oldsc = routeScore(nudgeCount(), None, CoreChain);
	postScore(oldsc);

	MultiSimplex<ResidueId> ms(this, parameterCount());
	ms.setStepSize(step);
	ms.supplyInfo(map);
	ms.run();
	zeroParameters();

	float newsc = routeScore(nudgeCount(), None, CoreChain);
	_bestScore = newsc;
	postScore(newsc);

	if (_finish) return false;

	struct RankedResidue
	{
		float score;
		ResidueId id;

		bool operator<(const RankedResidue &other) const
		{
			return score > other.score; // want descending
		}
	};

	std::set<RankedResidue> residues;

	for (auto it = map.begin(); it != map.end(); it++)
	{
		const ResidueId &id = it->first;
		_ids = {id};
		float sc = routeScore(nudgeCount(), None, CoreChain);
		residues.insert({sc, id});
	}

	OpSet<ResidueId> chosen;

	n = 0;
	for (const RankedResidue &rr : residues)
	{
		if (n >= 5) break;
		chosen.insert(rr.id);
		n++;
	}

	std::map<ResidueId, std::vector<int>> mini;
	for (const ResidueId &id : chosen)
	{
		mini[id] = map[id];
	}

	_ids = chosen;
	{
		MultiSimplex<ResidueId> ms(this, parameterCount());
		ms.setMaxRuns(20);
		ms.setStepSize(step);
		ms.supplyInfo(mini);
		for (int i = 0; i < 10; i++)
		{
			ms.run();
			zeroParameters();
		}
		if (_finish) return false;
	}

	_ids.clear();
		

	newsc = routeScore(nudgeCount(), None, CoreChain);
	_bestScore = newsc;
	postScore(newsc);
	std::cout << "here: " << oldsc << " to " << newsc << std::endl;
	return meaningfulUpdate(newsc, oldsc, 0.95);
}

bool PlausibleRoute::lateStageGradients()
{
	auto side_chain = [this](int idx) -> bool
	{
		return parameter(idx) && !parameter(idx)->coversMainChain() && !parameter(idx)->isConstrained();
	};

	GradientPath *path = gradients(side_chain);
	if (Route::_finish)
	{
		delete path; return false;
	}

	std::vector<Floats> current = save_current(path, _motions);
	ByResidueResult *best = nullptr;
	best = byResidueScore(nudgeCount());
	for (auto it = best->scores.begin(); it != best->scores.end(); it++)
	{
		it->second = FLT_MAX;
	}

	std::vector<float> alphas(best->scores.size(), 0);
	std::vector<float> steps(best->scores.size(), 0.2);
	std::vector<float> best_alphas(best->scores.size(), 0);

	std::map<int, int> indexed = indexed_motions(best, _motions);

	auto sum_score = [](ByResidueResult *score)
	{
		float sum = 0;
		for (auto it = score->scores.begin(); it != score->scores.end(); it++)
		{
			sum += it->second;
		}
		sum /= score->scores.size() * 2;
		return sum;
	};

	std::string desc;
	float biggest = 0;
	for (int j = 0; j < path->motion_idxs.size(); j++)
	{
		int p = path->motion_idxs[j];
		if (p >= 0 && p < motionCount())
		{
			const Floats &sines = path->grads[j];
			for (const float &s : sines)
			{
				if (fabs(s) > biggest)
				{
					biggest = fabs(s);
					desc = _motions.rt(p).desc();
				}
			}
		}
	}
	std::cout << "biggest: " << biggest << " for " << desc << std::endl;
	biggest *= 10;

	auto scores_for_alpha = [this, &biggest, &path, &current, &indexed]
		(const std::vector<float> &alphas) -> ByResidueResult *
		{

			for (int j = 0; j < path->motion_idxs.size(); j++)
			{
				int p = path->motion_idxs[j];
				if (p >= 0 && p < motionCount())
				{
					int a = indexed[p];
					float alpha = alphas[a];

					const Floats &sines = path->grads[j];
					for (int i = 0; i < sines.size(); i++)
					{
						float add = sines[i] * alpha / biggest * 10;
						if (add > 0.1 * biggest)
						{
							continue;
						}
						motion(p).wp._amps[i] = current[p][i] + add;
					}
				}
			}

			ByResidueResult *score;
			score = byResidueScore(nudgeCount());

			return score;
		};

	auto adjust_alphas = [this](std::vector<float> &alphas,
			std::vector<float> &best_alphas,
			std::vector<float> &steps,
			ByResidueResult *last, ByResidueResult *best)
		-> bool
		{
			auto jt = best->scores.begin();
			int idx = 0;
			bool another = false;
			for (auto it = last->scores.begin(); it != last->scores.end();
					it++)
			{
				float my_score = it->second;
				float best_score = jt->second;

				if (my_score < best_score)
				{
					/*
					   std::cout << it->first << ": " << my_score << " vs " <<
					   best_score << " with alpha " << " " << alphas[idx] << 
					   " step " << steps[idx] << std::endl;
					 */

					jt->second = my_score;
					best_alphas[idx] = alphas[idx];
				}
				else
				{
					//                             alphas[idx] = best_alphas[idx];
					steps[idx] *= 0.5;
				}

				if (steps[idx] > 1 / 12.f)
				{
					another = true;
				}

				jt++; idx++;
			}

			for (int i = 0; i < alphas.size() && another; i++)
			{
				alphas[i] += steps[i];
			}

			return another;
		};

	std::vector<float> chosen_alphas(best->scores.size(), 0);
	float best_score = FLT_MAX;

	int count = 0;
	while (count < 20)
	{
		ByResidueResult *next = scores_for_alpha(alphas);
		float score = sum_score(next);
		std::cout << "mixed alphas -> " << score << std::endl;
		if (score < best_score)
		{
			chosen_alphas = alphas;
			best_score = score;
		}

		bool another = adjust_alphas(alphas, best_alphas, steps, next, best);
		next->destroy();
		count++;
		if (!another) break;
	}

	ByResidueResult *next = scores_for_alpha(best_alphas);
	float alternative = sum_score(next);
	std::cout << "Best alphas combined: " << alternative << std::endl;
	if (best_score < alternative)
	{
		ByResidueResult *choice = scores_for_alpha(chosen_alphas);
		std::cout << "Better: " << sum_score(choice) << std::endl;
		choice->destroy();
	}

	best->destroy();
	next->destroy();

	return count > 1;
}

bool PlausibleRoute::applyGradients(const ValidateParam &validate)
{
	auto side_chain = [this](int idx) -> bool
	{
		return parameter(idx) && !parameter(idx)->coversMainChain();
	};

	GradientPath *path = gradients(doingSides() ? side_chain : validate);
	if (Route::_finish)
	{
		delete path;
		return false;
	}

	std::vector<Floats> current = save_current(path, _motions);

	float scale = 1;
	float alpha = 0;
	float step = 0.25;
	
	auto score_for_alpha = [this, scale, path, current]
	(const float &alpha) -> float
	{
		for (int j = 0; j < path->motion_idxs.size(); j++)
		{
			int p = path->motion_idxs[j]; // motion_idx
			if (p >= 0 && p < motionCount())
			{
				const Floats &sines = path->grads[j];
				for (int i = 0; i < sines.size(); i++)
				{
					float add = sines[i] * alpha * scale;
					motion(p).wp._amps[i] = current[p][i] + add;
				}
			}
		}

		float score = routeScore(nudgeCount(), None, CoreChain);
//		std::cout << alpha << " -> " << score << std::endl;
		return score;
	};

	float best_score = score_for_alpha(0);
	float first_score = best_score;
	float best_alpha = 0;

	int divisions = 0;
	while (divisions < 12)
	{
		float candidate = alpha + step;
		float score = score_for_alpha(candidate);
		
		if (score < best_score)
		{
			best_score = score;
			postScore(best_score);
			best_alpha = candidate;
			alpha = best_alpha;
			divisions = 0;
			step *= 1.5;
		}
		else if (score >= best_score)
		{
			divisions++;
			step /= 2;
		}
		
		if (Route::_finish)
		{
			break;
		}
	}
	
	best_score = score_for_alpha(best_alpha);
	_bestScore = best_score;
	postScore(best_score);
	
	path->destroy();
	delete path;

	if (_finish) return false;
	if (best_alpha < 1e-4) return false;
	if (first_score - best_score < 1e-4) return false;

	return true;
}

GradientPath *PlausibleRoute::gradients(const ValidateParam &validate,
                                        const CalcOptions &add_options,
                                        const CalcOptions &subtract_options)
{
	CalcOptions options = calcOptions(add_options, subtract_options);
	
	int order = doingQuadratic() ? 1 : 2;

	return submitGradients(options, order, validate, _hydrogenFreeSequences);
}

ByResidueResult *PlausibleRoute::byResidueScore(int steps, 
                                                const CalcOptions &add_options,
                                                const CalcOptions 
                                                &subtract_options)
{
	clearTickets();
	CalcOptions options = calcOptions(add_options, subtract_options);

	if (!(options & PerResidue && options & VdWClashes))
	{
		throw std::runtime_error("Using wrong options to call a per-residue "
		                         "score in PlausibleRoute.cpp");
	}

	submitValue(options, steps, nullptr);
	retrieve();
	_perResBin.releaseHorses();
	ByResidueResult *r = _perResBin.acquireObject();
	float total = 0;
	float c = 0;
	for (auto it = r->scores.begin(); it != r->scores.end(); it++)
	{
		total += it->second;
		c++;
	}
	total /= 2 * c;

	return r;
}

float PlausibleRoute::routeScore(int steps, const CalcOptions &add_options,
                                 const CalcOptions &subtract_options)
{
	clearTickets();
	CalcOptions extra_subtract = CalcOptions(subtract_options | PerResidue);
	CalcOptions options = calcOptions(add_options, extra_subtract);

	if (options & PerResidue && options & VdWClashes)
	{
		throw std::runtime_error("Using wrong function to call a per-residue "
		                         "score in PlausibleRoute.cpp");
	}

	float sc = 0;
	submitValue(options, steps, nullptr);
	retrieve();
	sc = _point2Score[0].deviations / (float)steps;
	float highest = _point2Score[0].highest_energy;
	float lowest = _point2Score[1].lowest_energy;
	_activationEnergy = highest - lowest;

	bool show = rand() % 5 == 0;
	if (show || true)
	{
		submitToShow(_chosenFrac);
		retrieve();
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

void PlausibleRoute::addFloatParameter(float *value, float step)
{
	_paramStarts.push_back(*value);
	_paramPtrs.push_back(value);
	_steps.push_back(step);
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
	_steps.clear();
	_ids.clear();
	
	for (size_t i = 0; i < idxs.size(); i++)
	{
		if (idxs[i] < 0)
		{
			continue;
		}
		
		_ids.insert(parameter(idxs[i])->residueId());
		WayPoints &wps = wayPoints(idxs[i]);
//		Motion &motion = _motions.storage(idxs[i]);
		
		float movement = fabs(destination(idxs[i]));
		float step = doingClashes() ? 2 : 1;

		addFloatParameter(&wps._amps[0], step);

		if (doingCubic())
		{
			addFloatParameter(&wps._amps[1], step);
		}
	}
	
	_bestScore = routeScore(nudgeCount());
	
	int runs = doingCubic() ? 20 : 10;
	
	_simplex->setMaxRuns(runs);
	_simplex->chooseStepSizes(_steps);
}

size_t PlausibleRoute::parameterCount(Engine *caller)
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
		    (doingClashes() && motion(i).locked > 0))
		{
			continue;
		}
		
		OpSet<int> single;
		single.insert(indexOfParameter(centre));

		single.filter(validate);
		
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

		float candidate = routeScore(flipNudgeCount(), None, Pairwise);

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

	if (doingClashes()) return false;

	_bestScore = routeScore(flipNudgeCount(), None, Pairwise);
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

	_bestScore = routeScore(flipNudgeCount(), None, Pairwise);
	postScore(_bestScore);
}


void PlausibleRoute::flipTorsionCycles(const ValidateParam &validate)
{
	if (_maxFlipTrial == 0) return;
	flipTorsionCycle(validate);
}

void PlausibleRoute::cycle()
{
	while (!lastJob())
	{
		for (PlausibleRoute::Task &task : _tasks)
		{
			time_t start = ::time(NULL);
			task();
			time_t end = ::time(NULL);
			time_t duration = end - start;
			int seconds = duration % 60;
			int minutes = (duration - seconds + 1) / 60;
			if (seconds > 0 || minutes > 0)
			{
				std::cout << "Cycle: " << minutes << "m " << seconds 
				<< "s." << std::endl;
			}
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

	auto fetch = [this, basis, lookup](const Coord::Get &get, int torsion_idx)
	{
		int mot_idx = lookup[torsion_idx];
		if (mot_idx < 0) return 0.f;

		float t = motion(mot_idx).interpolatedAngle(get(0));
		if (t != t) t = 0;
		return t;
	};

	handler->manager()->setTorsionFetcher(fetch);
}

void PlausibleRoute::zeroParameters()
{
	for (size_t i = 0; i < _paramPtrs.size(); i++)
	{
		float update_value = *(_paramPtrs[i]);
		_paramStarts[i] = update_value;
	}
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

std::map<OpSet<ResidueId>, float> PlausibleRoute::
	getMultiResult(const std::vector<float> &all,
	               MultiSimplex<OpSet<ResidueId>> *caller)
{
	assignParameterValues(all);
	
	int num = nudgeCount();
	if (rand() % 5 == 0 || true)
	{
		submitToShow(_chosenFrac);
		retrieve();
	}

	ByResidueResult *rr = byResidueScore(num);
	std::map<ResidueId, float> scores = rr->scores;
	std::map<OpSet<ResidueId>, float> converted = caller->blueprint();
	
	for (auto it = converted.begin(); it != converted.end(); it++)
	{
		for (const ResidueId &id : it->first)
		{
			it->second += scores[id];
		}
	}

	delete rr;
	return converted;
}

std::map<ResidueId, float> PlausibleRoute::
	getMultiResult(const std::vector<float> &all,
	               MultiSimplex<ResidueId> *caller)
{
	assignParameterValues(all);
	
	int num = nudgeCount();
	if (rand() % 5 == 0 || true)
	{
		submitToShow(_chosenFrac);
		retrieve();
	}

	ByResidueResult *rr = byResidueScore(num);
	std::map<ResidueId, float> scores = rr->scores;
	
	/*
	for (auto it = converted.begin(); it != converted.end(); it++)
	{
		for (const ResidueId &id : it->first)
		{
			it->second += scores[id];
		}
	}
	*/

	delete rr;
	return scores;
}

int PlausibleRoute::sendJob(const std::vector<float> &all, Engine *caller)
{
	assignParameterValues(all);
	float result = FLT_MAX;
	
	int num = nudgeCount();
	result = routeScore(num);
	
	int ticket = getNextTicket();
	setScoreForTicket(ticket, result);
	return ticket;
}

void PlausibleRoute::upgradeJobs()
{
	if (lastJob()) { return; }

	_jobLevel++;

	if (doingClashes() && !doingHydrogens())
	{
		_jobLevel++;
	}
	
	unlockAll();
	std::cout << "Job level now " << _jobLevel << "..." << std::endl;
}


void PlausibleRoute::refreshScores()
{
	_jobLevel = 0;
	_momentum = routeScore(nudgeCount());

	_jobLevel = 4;
	_clash = routeScore(nudgeCount(), None, TorsionEnergies);
	_vdwEnergy = _activationEnergy;
	routeScore(nudgeCount(), TorsionEnergies, VdWClashes);
	_torsionEnergy = _activationEnergy;

	std::cout << "Minimise momentum score " << _momentum << std::endl;
	std::cout << "Minimise clash score " << _clash << std::endl;
	std::cout << "VdW energy " << _vdwEnergy << std::endl;
	std::cout << "Torsion energy " << _torsionEnergy << std::endl;

	_activationEnergy = _vdwEnergy;
	
	_gotScores = true;
}
