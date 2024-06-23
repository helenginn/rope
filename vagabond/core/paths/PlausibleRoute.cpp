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
#include "paths/ResidueContacts.h"
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

void PlausibleRoute::rewindTorsions()
{
	for (int i = 0; i < motionCount(); i++)
	{
		if (!parameter(i)->coversMainChain())
		{
			continue;
		}

		std::vector<int> seq = getTorsionSequence
		(i, -1, [this](const int &idx)
		 {
			return parameter(idx)->coversMainChain();
		});

		float total = 0;
		for (int j = 0; j < seq.size(); j++)
		{
			float angle = motion(seq[j]).workingAngle();
			total += angle;
			
			if (angle > 150 && total > 480 && !motion(seq[j]).flip)
			{
				total -= 360;
				motion(seq[j]).flip = !motion(seq[j]).flip;
			}
			if (angle < -150 && total < -480 && !motion(seq[j]).flip)
			{
				total += 360;
				motion(seq[j]).flip = !motion(seq[j]).flip;
			}
			
			std::cout << angle << " " << total;
			std::cout << std::endl;
		}

		break;
	}
}

void PlausibleRoute::setup()
{
	Route::setup();
	prepareDestination();
	prepareTorsionFetcher(_resources.sequences);
	prepareAlignment();
	setTargets();

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
	float required_reduction = doingClashes() ? 0.5 : 0.001;
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
		flipTorsionCycles(large_main);
	};
	
	auto large_side = [this](int idx) -> bool
	{
		return validateTorsion(idx, 90, 360., false, true);
	};

	auto flip_side_chain = [this, large_side]()
	{
		flipTorsionCycles(large_side);
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
				if (_finish)
				{
					return;
				}
				if (!good)
				{
					if (amnesty < 16 && !doingClashes())
					{
						continue;
					}

					if (!_finish)
					{
						upgradeJobs();
					}

					float after = _bestScore;
					std::cout << before << " to " << after << std::endl;
					break;
				}

			}
		};
	};
	
	auto do_next = [this]()
	{
		if (doingClashes()) { return sideChainGradients(); }
		else { return applyGradients(); };
	};

	only_once flip_mains(flip_main_chain);
//	only_once flip_sides(flip_side_chain);
	_tasks.push_back(flip_mains);
	_tasks.push_back(check_gradients(do_next));
//	_tasks.push_back(flip_sides);

	return;
	_tasks.push_back([this]()
	                 {
		                repelMainChainAtomsFromWorstResidues();
		                return false;
		             });
}

void PlausibleRoute::setTargets(Instance *inst)
{
	std::map<Atom *, glm::vec3> atomStart;
	AtomGroup *grp = inst->currentAtoms();

	submitToShow(0.);
	retrieve();

	for (Atom *atom : grp->atomVector())
	{
		glm::vec3 d = atom->derivedPosition();
		atomStart[atom] = d;
	}

	submitToShow(1.);
	retrieve();

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

	std::map<ResidueId, std::vector<int>> map;
	int n = 0;
	for (int i = 0; i < motionCount(); i++)
	{
		ResidueTorsion &rt = residueTorsion(i);
		if (parameter(i) && 
		    (parameter(i)->coversMainChain() || parameter(i)->isConstrained()))
		{
			continue;
		}

		const ResidueId &local = rt.local_id();

		_ids = {local};
		float sc = routeScore(nudgeCount());
		residues.insert({sc, local});
	}

	OpSet<ResidueId> chosen;

	n = 0;
	for (const RankedResidue &rr : residues)
	{
		if (n >= num) break;
		chosen.insert(rr.id);
		std::cout << rr.id << " has score " << rr.score << std::endl;
		n++;
	}
	
	_ids.clear();

	return chosen;
}

bool PlausibleRoute::sideChainGradients()
{
	_paramPtrs.clear();
	_paramStarts.clear();
	_steps.clear();

	float step = 10;
	std::map<ResidueId, std::vector<int>> map;
	int n = 0;
	for (int i = 0; i < motionCount(); i++)
	{
		ResidueTorsion &rt = residueTorsion(i);
		if (!parameter(i) ||
		    (parameter(i)->coversMainChain() || parameter(i)->isConstrained()))
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

	float oldsc = routeScore(nudgeCount());
	postScore(oldsc);

	MultiSimplex<ResidueId> ms(this, parameterCount());
	ms.setStepSize(step);
	ms.supplyInfo(map);
	ms.run();
	zeroParameters();

	float newsc = routeScore(nudgeCount());
	_bestScore = newsc;
	postScore(newsc);

	if (_finish) return false;

	OpSet<ResidueId> chosen = worstSidechains(5);
	_ids = chosen;

	std::map<ResidueId, std::vector<int>> mini;
	for (const ResidueId &id : chosen)
	{
		mini[id] = map[id];
	}

	{
		MultiSimplex<ResidueId> ms(this, parameterCount());
//		ms.setMaxRuns(20);
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

	newsc = routeScore(nudgeCount());
	_bestScore = newsc;
	postScore(newsc);
	std::cout << "here: " << oldsc << " to " << newsc << std::endl;
	return meaningfulUpdate(newsc, oldsc, 0.95);
}

void PlausibleRoute::repelMainChainAtomsFromWorstResidues()
{
	if (!lastJob() || _repelCount >= 6)
	{
		return;
	}

	_repelCount++;
	AtomGroup *all = all_atoms();
//	OpSet<ResidueId> worst = worstSidechains(5);

	std::cout << "Repelling main chain atoms!" << std::endl;
	submitToShow(0.25);
	retrieve();

	all->writeToFile("compare.pdb");
	AtomGroup *all_contacts = new AtomGroup();
	AtomGroup *main = all->new_subset([](Atom *const &a)
	                                 {
		                                return a->isMainChain();
	                                 });

	float frac = 0.25;
	auto get_too_close = [&frac, this, main](Atom *const &a)
	{
		PairwiseDeviations *chosen = 
		pairwiseForSequences(_hydrogenFreeSequences);

		const glm::vec3 &p = a->derivedPosition();
		float radius = 2.5;

		for (Atom *const &other : main->atomVector())
		{
			if (other == a || other->bondsBetween(a, 4, false) > 0)
			{
				return false;
			}

			const glm::vec3 &q = other->derivedPosition();
			glm::vec3 diff = p - q;
			float l = glm::length(diff);
			if (l < radius)
			{
				bool too_close = false;
				for (size_t k = 0; k < a->bondTorsionCount(); k++)
				{
					if (a->bondTorsion(k)->hasAtom(other))
					{
						too_close = true;
					}
				}

				if (too_close)
				{
					return false;
				}

				std::cout << a->desc() << " vs " << other->desc() << 
				" --> " << radius - l << std::endl;

				float extra = (radius - l) * 5;
				chosen->addWaypoint(a, other, frac, extra);

				diff *= radius / l;
//				glm::vec3 new_pos = q + diff;
//				other->setDerivedPosition(new_pos);

				return true;
			}
		}

		return false;
	};

	AtomGroup *contacts = main->new_subset(get_too_close);
	delete contacts;
	delete main;
	delete all_contacts;
	delete all;
	
	_jobLevel = 0;

	if (lastJob())
	{
		finish();
	}
}

bool PlausibleRoute::applyGradients()
{
	auto side_chain = [this](int idx) -> bool
	{
		return parameter(idx) && !parameter(idx)->coversMainChain();
	};

	GradientPath *path = gradients(doingSides() ? side_chain : ValidateIndex{});

	if (Route::_finish)
	{
		delete path;
		return false;
	}

	std::vector<Floats> current = save_current(path, _motions);

	for (int j = 0; j < path->motion_idxs.size(); j++)
	{
		int p = path->motion_idxs[j]; // motion_idx
		const Floats &sines = path->grads[j];
	}

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

		float score = routeScore(nudgeCount());
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
			return false;
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

GradientPath *PlausibleRoute::gradients(const ValidateIndex &validate,
                                        const CalcOptions &add_options,
                                        const CalcOptions &subtract_options)
{
	CalcOptions options = calcOptions(add_options, subtract_options);
	
	int order = doingQuadratic() ? 1 : _order;

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

	if (_gui)
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

bool PlausibleRoute::flipTorsions(const ValidateIndex &validate)
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

void PlausibleRoute::flipTorsionCycle(const ValidateIndex &validate)
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


void PlausibleRoute::flipTorsionCycles(const ValidateIndex &validate)
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

	auto fetch = [this, basis, lookup](const Coord::Get &get, int torsion_idx)
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

	ByResidueResult *rr = byResidueScore(num);
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

	_jobLevel++;

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
