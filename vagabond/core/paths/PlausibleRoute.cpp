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
#include "paths/PlausibleRoute.h"
#include "BondCalculator.h"
#include "MultiSimplex.h"
#include "GradientTerm.h"
#include "BondSequence.h"
#include "Polymer.h"
#include "TorsionData.h"
#include "PairwiseDeviations.h"
#include "BondSequenceHandler.h"
#include <vagabond/utils/maths.h>
#include "Grapher.h"
#include "paths/Targets.h"
#include <vagabond/c4x/Cluster.h>
#include <gemmi/elem.hpp>

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
	prepareAlignment();

	if (_isNew)	
	{
		bestGuessTorsions()();
	}
	
	prepareJobs();
	postScore(routeScore(nudgeCount()));
}

bool PlausibleRoute::meaningfulUpdate(float new_score, float old_score,
                                      float threshold)
{
	bool good_ratio = (new_score < threshold * old_score);
	float required_reduction = doingClashes() ? 1.0 : 0.001;
	bool bad_reduction = (old_score - new_score < required_reduction);

	return good_ratio && !bad_reduction;
}

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

	auto flip_main_chain = [this, large_main]()
	{
		flipTorsionCycle(large_main);
	};
	
	auto large_side = [this](int idx) -> bool
	{
		return validateTorsion(idx, 90, 360., false, true);
	};

	auto flip_side_chain = [this, large_side]()
	{
		flipTorsionCycle(large_side);
	};
	
	auto check_gradients = [this]
	(const std::function<bool()> &gradient_type)
	{
		return [this, gradient_type]()
		{
			bool good = true;
			float before = routeScore(nudgeCount());
			int amnesty = 0;
			while (true)
			{
				amnesty++;
				good = gradient_type();

				_ids.clear();
				if (_finish)
				{
					return;
				}

				if (!good)
				{
					if (amnesty < 1 && !doingClashes())
					{
						continue;
					}

					if (!_finish)
					{
						upgradeJobs();
						
						if (lastJob())
						{
							finish();
							break;
						}
					}

					float after = _bestScore;
					std::cout << before << " to " << after << std::endl;
					break;
				}

			}
		};
	};
	
	int start = (_order <= 2 ? 1 : 0);
	auto increase_order_on_success = [this, start]
	(const std::function<bool(int)> &job, int max)
	{
		int i = start;
		bool improved = false;
		while (i < max)
		{
			bool this_time = job(i);
			if (_finish) return false;
			improved |= this_time;
			if (i < max - 1)
			{
				i++;
			}
			else if (!this_time)
			{
				break;
			}
		}
		return improved;
	};

	auto do_next = [this, increase_order_on_success]()
	{
		if (doingClashes())
		{
			return
			increase_order_on_success([this](int i)
			                          { return sideChainGradients(i); },
			                          2);
		}
		else
		{
			bool success = applyGradients();
			return success;
		};
	};
	
	only_once flip_mains(flip_main_chain);
	only_once flip_sides(flip_side_chain);
	_tasks.push_back(flip_mains);
	_tasks.push_back(check_gradients(do_next));
//	_tasks.push_back(flip_sides);
}

void PlausibleRoute::setTargets()
{
	Targets targets(this, pairs());
	targets();
	
	updateAtomFetch(_resources.sequences);
	updateAtomFetch(_hydrogenFreeSequences);
	prepareEnergyTerms();
	prepareTorsionFetcher(_hydrogenFreeSequences);
}

PlausibleRoute::CalcOptions 
PlausibleRoute::calcOptions(const CalcOptions &add_options,
                                        const CalcOptions &subtract_options)
{
	CalcOptions options = Pairwise;

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

std::vector<float> save_current(GradientPath *path, const RTMotion &motions,
                                 int to_order)
{
	std::vector<float> current = std::vector<float>(motions.size() * to_order);

	for (int i = 0; i < path->motion_idxs.size(); i++)
	{
		int p = path->motion_idxs[i];
		if (p >= 0 && p < motions.size())
		{
			for (int j = 0; j < to_order; j++)
			{
				current[p * to_order + j] = motions.storage(p).wp._amps[j];
			}
		}
	}

	return current;
}

OpSet<ResidueId> PlausibleRoute::worstSidechains(int num)
{
	struct RankedResidue
	{
		float score;
		ResidueId id;

		bool operator<(const RankedResidue &other) const
		{
			return score > other.score && id != other.id; // want descending
		}
	};

	std::set<RankedResidue> residues;
	
	_ids.clear();
	ByResidueResult *rr = byResidueScore(nudgeCount());
	std::map<ResidueId, float> highests = rr->activations;
	delete rr;

	for (auto it = highests.begin(); it != highests.end(); it++)
	{
		const ResidueId &local = it->first;
		residues.insert({it->second, local});
	}

	OpSet<ResidueId> chosen;

	int n = 0;
	float sum = 0;
	_lemons.clear();

	for (const RankedResidue &rr : residues)
	{
		if (n >= num) break;
		chosen.insert(rr.id);
		if (doingClashes())
		{
			_lemons.push_back({rr.id, rr.score});
		}
		sum += rr.score;
		n++;
	}
	
	for (auto &lemon : _lemons)
	{
		lemon.second /= sum;
	}
	
	if (_lemons.size())
	{
		Route::sendResponse("lemon", nullptr);
	}
	
	_ids.clear();

	return chosen;
}

bool PlausibleRoute::sideChainGradients(int order)
{
	_paramPtrs.clear();
	_paramStarts.clear();
	_steps.clear();

	OpSet<ResidueId> chosen = worstSidechains(5);

	float step = doingClashes() ? 10 : 2.0;
	std::map<ResidueId, std::vector<int>> map;
	int n = 0;
	for (int i = 0; i < motionCount(); i++)
	{
		ResidueTorsion &rt = residueTorsion(i);
		if (!parameter(i) || (parameter(i)->isTorsion() &&
		    (parameter(i)->coversMainChain() || parameter(i)->isConstrained())))
		{
			continue;
		}
		
		if (!doingClashes() && (parameter(i)->atom(0)->elementSymbol() == "H" ||
		    parameter(i)->atom(3)->elementSymbol() == "H"))
		{
			// trying to do this on momentum isn't very successful.
			continue;
		}

		const ResidueId &local = rt.local_id();
		WayPoints &wps = wayPoints(i);

		for (int j = 0; j <= order; j++)
		{
			addFloatParameter(&wps._amps[j], step);
			map[local].push_back(n);
			n++;
		}
	}

	float oldsc = routeScore(nudgeCount());
	postScore(oldsc);

	installAllResidues();
	MultiSimplex<ResidueId> ms(this, parameterCount());
	ms.setStepSize(step);
	ms.supplyInfo(map);
	ms.run();
	zeroParameters();
	_ids.clear();

	float newsc = routeScore(nudgeCount());
	_bestScore = newsc;
	postScore(newsc);

	if (_finish) return false;

	chosen = worstSidechains(5);
	_ids = chosen;

	std::map<ResidueId, std::vector<int>> mini;
	for (const ResidueId &id : chosen)
	{
		mini[id] = map[id];
	}

	{
		if (!doingClashes()) step = 1;
		MultiSimplex<ResidueId> ms(this, parameterCount());
		ms.setStepSize(step);
		ms.supplyInfo(mini);
		for (int i = 0; i < 10; i++)
		{
			_ids = chosen;
			ms.run();
			zeroParameters();
		}

		if (_finish) return false;
	}

	_ids.clear();

	newsc = routeScore(nudgeCount());
	_bestScore = newsc;
	postScore(newsc);
	std::cout << "here: " << oldsc << " to " << newsc << std::endl;
	return meaningfulUpdate(newsc, oldsc, 0.90);
}

template <typename JobOnTerm>
auto do_on_each_path_component(PlausibleRoute *pr, 
                               GradientPath *path, const JobOnTerm &job)
{
	for (int j = 0; j < path->motion_idxs.size(); j++)
	{
		int p = path->motion_idxs[j]; // motion_idx
		if (p >= 0 && p < pr->motionCount())
		{
			const Floats &sines = path->grads[j];
			for (int i = 0; i < sines.size(); i++)
			{
				int n = p * pr->currentOrder() + i;
				job(p, i, n);
			}
		}
	}

}

float PlausibleRoute::evaluateMomentum(const lbfgsfloatval_t *x)
{
	do_on_each_path_component(this, _path,
	[this, x](int p, int i, int n)
	{
		motion(p).wp._amps[i] = x[n];
	});

	float score = routeScore(nudgeCount());
	
	if (score < _bestScore)
	{
		_bestScore = score;
		postScore(score);
	}

	return score;
}

void PlausibleRoute::prepareGradients(lbfgsfloatval_t *g)
{
	const auto side_chain = [this](int idx) -> bool
	{
		return ((parameter(idx) && !parameter(idx)->coversMainChain()) ||
		        !parameter(idx)->isTorsion()) ;
	};

	delete _path;
	GradientPath *path = gradients(doingSides() ? side_chain : ValidateIndex{});
	_path = path;

	for (int j = 0; j < _path->motion_idxs.size(); j++)
	{
		int p = _path->motion_idxs[j]; // motion_idx
		if (p >= 0 && p < motionCount())
		{
			const Floats &sines = _path->grads[j];
			for (int i = 0; i < sines.size(); i++)
			{
				int n = p * currentOrder() + i;
				g[n] = sines[i];
			}
		}
	}
}

lbfgsfloatval_t evaluateLbfgs(void *instance, 
                                 const lbfgsfloatval_t *x, lbfgsfloatval_t *g,
                                 const int n, const lbfgsfloatval_t step)
{

	PlausibleRoute *pr = static_cast<PlausibleRoute *>(instance);
	float result = pr->evaluateMomentum(x);
	pr->prepareGradients(g);
	return result;
}

GradientPath *PlausibleRoute::gradients(const ValidateIndex &validate,
                                        const CalcOptions &add_options,
                                        const CalcOptions &subtract_options)
{
	CalcOptions options = calcOptions(add_options, subtract_options);
	
	int order = _jobOrder + 1;

	return submitGradients(options, order, validate, _hydrogenFreeSequences);
}

bool PlausibleRoute::applyGradients()
{
	GradientPath *path = gradients(ValidateIndex{});

	if (Route::_finish)
	{
		delete path;
		return false;
	}

	lbfgs_parameter_t params;
	
	lbfgs_parameter_init(&params);
	params.max_iterations = 10;
	params.max_linesearch = 10;

	float startScore = routeScore(nudgeCount());
	_bestScore = startScore;

	_path = path;
	std::vector<float> current = save_current(path, _motions, currentOrder());
	float endScore = 0;
	lbfgs(current.size(), &current[0], &endScore, &evaluateLbfgs,
	      nullptr, this, &params);

	sideChainGradients(currentOrder());
	endScore = routeScore(nudgeCount());
	
	delete _path; _path = nullptr;
	bool meaningful = endScore < startScore - 0.001;
	std::cout << "n = " << current.size() << ", " << 
	startScore << " --> " << endScore << " ";
	std::cout << (meaningful ? "(meaningful)" : "(meaningless)") << std::endl;
	return meaningful;
}

ByResidueResult *PlausibleRoute::byResidueScore(int steps, 
                                                const CalcOptions &add_options,
                                                const CalcOptions 
                                                &subtract_options)
{
	clearTickets();
	CalcOptions options = calcOptions(CalcOptions(add_options | PerResidue),
	                                  subtract_options);

	if (!(options & PerResidue))
	{
		throw std::runtime_error("Using wrong options to call a per-residue "
		                         "score in PlausibleRoute.cpp");
	}

	submitValue(options, steps, nullptr);
	retrieve();
	_perResBin.releaseHorses();
	ByResidueResult *r = _perResBin.acquireObject();
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
	_energy = highest - lowest;

	if (_gui)
	{
		submitToShow(_chosenFrac);
		retrieve();
	}
	
	clearTickets();
	return sc;
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
	
	if ((p && p->isConstrained()) ||
	    (!p || (mains_only && !p->coversMainChain()) ||
	    (sides_only && p->coversMainChain())))
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

size_t PlausibleRoute::parameterCount(Engine *caller)
{
	return _paramPtrs.size();
}

void print(std::vector<int> &flips)
{
	std::cout << flips.size() << ": ";
	for (size_t j = 0; j < flips.size(); j++)
	{
		std::cout << (flips[j] ? "Y" : "n");
	}
}

std::vector<int>
PlausibleRoute::getTorsionSequence(int idx, int max,
                                   const ValidateIndex &validate)
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
	
	while (g && (max < 0 || count < max))
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

bool PlausibleRoute::flipTorsion(const ValidateIndex &validate, int idx)
{
	std::vector<int> idxs = getTorsionSequence(idx, _maxFlipTrial, validate);
	
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

		float candidate = routeScore(flipNudgeCount());//, None, Pairwise);

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

bool PlausibleRoute::flipTorsions(const ValidateIndex &validate)
{
	bool changed = false;

	if (doingClashes()) return false;

	_bestScore = routeScore(flipNudgeCount());//, None, Pairwise);
	
	for (size_t i = 0; i < motionCount(); i++)
	{
		if (Route::_finish)
		{
			return false;
		}

		clickTicker();
		
		if (!validate(i))
		{
			continue;
		}

		bool changed_last = flipTorsion(validate, i);
		changed |= changed_last;
	}
	
	finishTicker();

	return changed;
}

void PlausibleRoute::flipTorsionCycle(const ValidateIndex &validate)
{
	if (_maxFlipTrial == 0) return;
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
		if (_motionFilter && !_motionFilter(mot_idx)) return 0.f;

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

void PlausibleRoute::finishedKey(const ResidueId &key)
{
	_ids.erase(key);
}

std::map<ResidueId, float> PlausibleRoute::
	getMultiResult(const std::vector<float> &all,
	               MultiSimplex<ResidueId> *caller)
{
	assignParameterValues(all);
	
	int num = nudgeCount();
	if (_gui)
	{
		submitToShow(_chosenFrac);
		retrieve();
	}
	
	ByResidueResult *rr = nullptr;
	if (doingClashes())
	{
		rr = byResidueScore(num);
	}
	else
	{
		rr = byResidueScore(num, NoHydrogens, VdWClashes);
	}

	std::map<ResidueId, float> scores = rr->scores;

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

	if (lastOrder() && _jobLevel == 0)
	{
		_jobLevel++;
		_jobOrder = _order;
	}
	else if (_jobLevel == 0)
	{
		_jobOrder++;
	}
	else
	{
		_jobLevel++;
	}

	std::cout << "Job level now " << _jobLevel << ", order " << _jobOrder 
	<< std::endl;
}

Contacts PlausibleRoute::contactMap()
{
	clearTickets();
	CalcOptions options = calcOptions(CalcOptions(VdWClashes | ContactMap),
	                                  None);
	submitValue(options, nudgeCount(), nullptr);
	
	Contacts contacts{};
	auto handle_results = [&contacts](Result *r)
	{
		contacts = r->contacts;
	};

	retrieve(handle_results);
	return contacts;
}

void PlausibleRoute::refreshScores()
{
	setFirstJob();
	_momentum = routeScore(nudgeCount());

	setLastJob();
	_clash = routeScore(nudgeCount(), None, TorsionEnergies);
	_vdwEnergy = _energy;
	routeScore(nudgeCount(), TorsionEnergies, VdWClashes);
	_torsionEnergy = _energy;

	std::cout << "Minimise momentum score " << _momentum << std::endl;
	std::cout << "Minimise clash score " << _clash << std::endl;
	std::cout << "VdW energy " << _vdwEnergy << std::endl;
	std::cout << "Torsion energy " << _torsionEnergy << std::endl;

	_activationEnergy = _vdwEnergy;
	
	_gotScores = true;
}
