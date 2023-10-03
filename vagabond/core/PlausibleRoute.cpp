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

#include "PlausibleRoute.h"
#include "Polymer.h"
#include "MetadataGroup.h"
#include <vagabond/utils/maths.h>
#include "Grapher.h"
#include <vagabond/c4x/Cluster.h>

PlausibleRoute::PlausibleRoute(Instance *inst, TorsionCluster *cluster,
                               int dims)
: Route(inst, cluster, dims)
{
	_threads = 4;
	_maximumCycles = 5;
}

void PlausibleRoute::setup()
{
	Route::setup();
	prepareDestination();
	connectParametersToDestination();
	setTargets();
}

void PlausibleRoute::setTargets()
{
	std::map<Atom *, glm::vec3> atomStart;

	twoPointProgression();
	submitJobAndRetrieve(0);

	for (Atom *atom : _fullAtoms->atomVector())
	{
		glm::vec3 d = atom->derivedPosition();
		atomStart[atom] = d;
	}

	submitJobAndRetrieve(1);

	for (Atom *atom : _fullAtoms->atomVector())
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
	calculateProgression(steps);
	clearTickets();

	float cumulative = 0;
	for (size_t i = 0; i < pointCount(); i++)
	{
		float rnd = rand() / (double)RAND_MAX;
		
		if (!_updateAtoms) rnd = 1.; // don't show

		if (!forceField)
		{
			submitJob(i, rnd < 0.01);
		}
		else
		{
			submitJob(i, true, true);
		}
	}
	
	retrieve();

	for (size_t i = 0; i < pointCount(); i++)
	{
		float contribution = (forceField ? score(i) : deviation(i));
		cumulative += contribution;
	}
	
	clearTickets();
	
	return cumulative / (float)steps;
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

bool PlausibleRoute::validateMainTorsion(int i, bool over_mag)
{
	if (_mainsOnly && !parameter(i)->coversMainChain())
	{
		return false;
	}
	
	float magnitude = fabs(destination(i));
	if (over_mag && magnitude < _magnitudeThreshold)
	{
		return false;
	}

	return true;
}

void PlausibleRoute::activateWaypoints(bool all)
{
	for (int idx : _activeTorsions)
	{
		if (idx < 0)
		{
			continue;
		}

		WayPoints &wps = wayPoints(idx);
		for (size_t j = 1; j < wps.size() - 1; j++)
		{
			wps[j].setChanged(true);
		}
	}
	
	if (all)
	{
		for (size_t i = 0; i < wayPointCount(); i++)
		{
			WayPoints &wps = wayPoints(i);
			for (size_t j = 1; j < wps.size() - 1; j++)
			{
				wps[j].setChanged(true);
			}
		}
	}
}

int PlausibleRoute::countTorsions()
{
	int count = 0;
	for (size_t i = 0; i < motionCount(); i++)
	{
		if (validateMainTorsion(i))
		{
			count++;
		}
	}
	
	return count;
}

void PlausibleRoute::prepareAnglesForRefinement(std::vector<int> &idxs)
{
	if (_simplex)
	{
		delete _simplex;
		_simplex = nullptr;
	}
	
	_simplex = new SimplexEngine(this);

	_activeTorsions = idxs;
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
		for (size_t j = 1; j < wps.size() - 1; j++)
		{
			WayPoint &wp = wps[j];
			
			_paramStarts.push_back(wp.fraction());
			_paramStarts.push_back(wp.progress());

			_paramPtrs.push_back(&wp.fraction_var());
			_paramPtrs.push_back(&wp.progress_var());

			steps.push_back(_stepSize);
			steps.push_back(_stepSize);
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

	activateWaypoints(true);
	_bestScore = routeScore(_nudgeCount);

	_simplex->start();

	bool changed = false;

	float bs = _simplex->bestScore();
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

std::vector<int> PlausibleRoute::getIndices(const std::set<Parameter *> &related)
{
	std::vector<int> idxs;
	
	for (Parameter *p : related)
	{
		int idx = indexOfParameter(p);
		if (idx >= 0 && validateMainTorsion(idx))
		{
			idxs.push_back(idx);
		}
	}

	return idxs;
}

int PlausibleRoute::nudgeWaypoints()
{
	int changed = 0;
	startTicker("Nudging waypoints (" + f_to_str(_magnitudeThreshold, 1)
	            + " degrees +)");

	float start = routeScore(flipNudgeCount());

	for (size_t i = 0; i < motionCount(); i++)
	{
		if (Route::_finish)
		{
			return false;
		}

		clickTicker();

		if (!validateMainTorsion(i))
		{
			continue;
		}
		
		Parameter *centre = parameter(i);
		std::set<Parameter *> related = centre->relatedParameters();
		std::vector<int> single;
		single.push_back(indexOfParameter(centre));
		
		std::vector<int> torsionIdxs = getIndices(related);
		
		bool result = simplexCycle(single);
		
		changed += (result ? 1 : 0);
	}

	postScore(_bestScore);
	finishTicker();
	
	if (_magnitudeThreshold < 6 && _bestScore / start > 0.9 && splitCount() < 2)
	{
		splitWaypoints();
	}

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


std::vector<int> PlausibleRoute::getTorsionSequence(int start, int max, 
                                                    float maxMag)
{
	AtomGraph *g = grapherForTorsionIndex(start);

	std::vector<int> idxs;
	if (!g) return idxs;

	int count = 0;
	idxs.push_back(start);
	
	while (g && count < max)
	{
		for (size_t j = 0; j < g->children.size(); j++)
		{
			Atom *a = g->children[j]->atom;
			AtomGraph *candidate = grapher().graph(a);
			int n = indexOfParameter(candidate->torsion);
			if ((n < 0) || !validateMainTorsion(n, true))
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


bool PlausibleRoute::flipTorsion(int idx)
{
	std::vector<int> idxs = getTorsionSequence(idx, 5, 30.f);
	
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

bool PlausibleRoute::flipTorsions(bool main)
{
	if (!_flipTorsions)
	{
		return false;
	}
	
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
		
		if (!validateMainTorsion(i))
		{
			continue;
		}

		float magnitude = fabs(destination(i));
		if (magnitude < 30.f)
		{
			continue;
		}
		
		changed |= flipTorsion(i);
	}
	
	finishTicker();

	return changed;
}

void PlausibleRoute::flipTorsionCycle(bool main)
{
	int count = 0;

	while (flipTorsions(main) && count < 100)
	{
		if (Route::_finish)
		{
			return;
		}
		
		count++;
	}
}


void PlausibleRoute::flipTorsionCycles()
{
	flipTorsionCycle(true);
}

void PlausibleRoute::nudgeWayPointCycles()
{
	int count = 0;
	int big_count = 0;
	_bestScore = routeScore(_nudgeCount);
	
	while (big_count < _maximumCycles * 4)
	{
		if (Route::_finish)
		{
			return;
		}
		
		int total = countTorsions();
		int change = nudgeWaypoints();

		float frac = (float)change / (float)total;
		count++;
		big_count++;

		if (_magnitudeThreshold < 40)
		{
			_stepSize = 0.01;
		}
		
//		std::cout << count << " / " << _maximumCycles << std::endl;
		if (frac < 0.25 || total == 0 || count >= _maximumCycles)
		{
			_magnitudeThreshold /= 2;
			count = 0;
		}
		
		if (!Route::_finish && 
		    _magnitudeThreshold < _minimumMagnitude && frac < 0.25)
		{
			_magnitudeThreshold = 90;
			break;
		}
	}
}

void PlausibleRoute::doCycle()
{
	if (_jobNum == 0)
	{
		flipTorsionCycles();
	}
	else
	{
		nudgeWayPointCycles();
	}
	
	_jobNum++;
	_cycles--;

	if (Route::_finish)
	{
		return;
	}
}

void PlausibleRoute::cycle()
{
	setTargets();
	
	while (_cycles != 0)
	{
		doCycle();

		if (Route::_finish)
		{
			return;
		}
	}

	flipTorsionCycle(false);
}

void PlausibleRoute::doCalculations()
{
	printWaypoints();

	if (!Route::_finish)
	{
		cycle();
	}

	
	finishTicker();
	prepareForAnalysis();
	Route::_finish = false;
}

float PlausibleRoute::getLinearInterpolatedTorsion(int i, float frac)
{
	float angle = getTorsionAngle(i);
	float progress = wayPoints(i).progress(frac);

	float new_angle = angle * progress;

	return new_angle;
}

void PlausibleRoute::addLinearInterpolatedPoint(float frac)
{
	Point point(motionCount(), 0);

	for (size_t i = 0; i < motionCount(); i++)
	{
		float t = getLinearInterpolatedTorsion(i, frac);
		point[i] = t;
	}
	
	addPoint(point);
}

float PlausibleRoute::getPolynomialInterpolatedTorsion(PolyFit &fit, int i,
                                                       float frac)
{
	float angle = getTorsionAngle(i);
	float prop = WayPoints::getPolynomialInterpolatedFraction(fit, frac);
	
	return angle * prop;
}

std::vector<PlausibleRoute::PolyFit> PlausibleRoute::polynomialFits()
{
	std::vector<PolyFit> fits;
	fits.reserve(motionCount());
	activateWaypoints();

	for (size_t i = 0; i < motionCount(); i++)
	{
		PolyFit pf = wayPoints(i).polyFit();
		fits.push_back(pf);
	}
	
	return fits;
}

void PlausibleRoute::addPolynomialInterpolatedPoint(std::vector<PolyFit> &fits,
                                                    float frac)
{
	Point point(motionCount(), 0);

	for (size_t i = 0; i < motionCount(); i++)
	{
		float t = getPolynomialInterpolatedTorsion(fits[i], i, frac);
		point[i] = t;
	}

	addPoint(point);
}

void PlausibleRoute::calculatePolynomialProgression(int steps)
{
	clearPoints();
	float frac = 0;
	float step = 1 / (float)steps;
	std::vector<PolyFit> fits = polynomialFits();

	for (size_t i = 0; i <= steps; i++)
	{
		addPolynomialInterpolatedPoint(fits, frac);
		frac += step;
	}
}

void PlausibleRoute::twoPointProgression()
{
	clearPoints();
	addLinearInterpolatedPoint(0);
	addLinearInterpolatedPoint(1);

}

void PlausibleRoute::calculateLinearProgression(int steps)
{
	clearPoints();
	float frac = 0;
	float step = 1 / (float)steps;

	for (size_t i = 0; i <= steps; i++)
	{
		addLinearInterpolatedPoint(frac);
		frac += step;
	}
}

void PlausibleRoute::calculateProgression(int steps)
{
	switch (type())
	{
		case Linear:
		calculateLinearProgression(steps);
		break;
		
		case Polynomial:
		calculatePolynomialProgression(steps);
		break;
		
		default:
		break;
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

bool PlausibleRoute::validateWayPoint(const WayPoints &wps)
{
	float tolerance = 1 / (float)_nudgeCount;

	for (size_t i = 0; i < wps.size() - 1; i++)
	{
		if (wps.at(i).progress() > wps.at(i + 1).progress() - tolerance)
		{
			return false;
		}
	}

	return true;
}

bool PlausibleRoute::validateWayPoints()
{
	for (size_t i = 0; i < _activeTorsions.size(); i++)
	{
		const WayPoints &wps = wayPoints(_activeTorsions[i]);
		if (!validateWayPoint(wps))
		{
			return false;
		}
	}

	return true;
}

int PlausibleRoute::sendJob(const std::vector<float> &all)
{
	assignParameterValues(all);
	bool valid = validateWayPoints();
	float result = FLT_MAX;
	
	if (valid)
	{
		result = routeScore(_nudgeCount);
	}
	
	int ticket = getNextTicket();
	setScoreForTicket(ticket, result);
	return ticket;
}

void PlausibleRoute::prepareForAnalysis()
{
	float result = routeScore(_nudgeCount);
	postScore(result);
	calculateProgression(200);
	
	for (size_t i = 0; i < pointCount(); i++)
	{
		submitJob(i, true, true);
	}
	
	retrieve();
	clearTickets();
}

void PlausibleRoute::splitWaypoint(int idx)
{
	WayPoints &wps = wayPoints(idx);
	int count = wps.size();
	
	if (count > _splitCount + 2)
	{
		return;
	}
	
	wps.split();
	return;
}

void PlausibleRoute::splitWaypoints()
{
	_splitCount++;
	
	for (size_t i = 0; i < wayPointCount(); i++)
	{
		splitWaypoint(i);

	}
}

void PlausibleRoute::printWaypoints()
{
	return;
	std::cout << ",start,end,";
	std::cout << std::endl;

	for (size_t i = 0; i < motionCount(); i++)
	{
		Parameter *t = parameter(i);
		
		if (!t->coversMainChain())
		{
			continue;
		}
		
		float start = t->value();
		float diff = getTorsionAngle(i);
		
		if (t->isTorsion())
		{
			BondTorsion *tmp = static_cast<BondTorsion *>(t);
			std::cout << tmp->atom(1)->desc() << ":" << tmp->desc() << ",";
		}
		else
		{
			std::cout << t->desc() << ",";
		}
		std::cout << start << "," << start + diff << ",";
		
		PolyFit fit = wayPoints(i).polyFit();
		
		for (size_t j = 0; j < wayPoints(i).size(); j++)
		{
			std::cout << "(" << wayPoints(i)[j].fraction() << ",";
			std::cout << wayPoints(i)[j].progress() << "),";
		}
		std::cout << std::endl;
	}

}
