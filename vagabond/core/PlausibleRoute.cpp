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
#include "Molecule.h"
#include "BondSequence.h"
#include "MetadataGroup.h"
#include <vagabond/c4x/Cluster.h>
#include <vagabond/utils/polyfit.h>

PlausibleRoute::PlausibleRoute(Molecule *mol, Cluster<MetadataGroup> *cluster,
                               int dims)
: Route(mol, cluster, dims)
{
	_maxJobRuns = 6;
}

void PlausibleRoute::setup()
{
	if (_rawDest.size() == 0 && _destination.size() == 0)
	{
		throw std::runtime_error("No destination set for route");
	}

	Route::setup();
	prepareDestination();
	
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

	for (BondCalculator *calc : _calculators)
	{
		grapher().refreshTargets(calc);
	}
}

void PlausibleRoute::populateWaypoints()
{
	_wayPoints.clear();
	_flips.clear();
	_mask = std::vector<bool>(_destination.size(), true);
	
	for (size_t i = 0; i < _destination.size(); i++)
	{
		WayPoints wp;
		wp.push_back(WayPoint::startPoint());
		_flips.push_back(false);

		if (fabs(_destination[i]) < 30)
		{
			wp.push_back(WayPoint(0.33, 0.33));
			wp.push_back(WayPoint(0.66, 0.66));
		}
		else
		{
			wp.push_back(WayPoint::midPoint());
		}

		wp.push_back(WayPoint::endPoint());

		_wayPoints[i] = wp;
	}

}

void PlausibleRoute::prepareDestination()
{
	if (_cluster == nullptr)
	{
		return;
	}

	const std::vector<ResidueTorsion> &list = _cluster->dataGroup()->headers();
	std::vector<bool> found(list.size(), false);
	Point trial;

	for (BondCalculator *calc : _calculators)
	{
		TorsionBasis *basis = calc->sequence()->torsionBasis();
		trial = Point(basis->torsionCount(), 0);
		
		int count = 0;
		_torsions.clear();
		
		for (size_t i = 0; i < basis->torsionCount(); i++)
		{
			BondTorsion *t = basis->torsion(i);
			float v = _molecule->valueForTorsionFromList(t, list, _rawDest, found);
			_torsions.push_back(t);
			
			if (v == v)
			{
				count++;
				trial[i] = v;
			}
		}
		
		if (count > 0)
		{
			break;
		}
	}
	
	_destination = trial;
	
	populateWaypoints();

}

float PlausibleRoute::momentumScore(int steps)
{
	calculateProgression(steps);
	clearTickets();

	float cumulative = 0;
	for (size_t i = 0; i < pointCount(); i++)
	{
		float rnd = rand() / (double)RAND_MAX;
		submitJob(i, rnd < 0.01);
	}
	
	retrieve();

	for (size_t i = 0; i < pointCount(); i++)
	{
		float contribution = score(i);
		float frac = i / (float)(pointCount() - 1);
		cumulative += contribution;
	}
	
	clearTickets();
	
	return cumulative / (float)steps;
}

void PlausibleRoute::startTicker(std::string tag, int d)
{
	if (d < 0)
	{
		d = dims();
	}

	HasResponder<Responder<Route>>::sendResponse("progress_" + tag, 
	                                             &d);
}

void PlausibleRoute::postScore(float score)
{
	HasResponder<Responder<Route>>::sendResponse("score", &score);
}

bool PlausibleRoute::validateMainTorsion(int i)
{
	if (_mainsOnly && !_torsions[i]->coversMainChain())
	{
		return false;
	}
	
	if (!usingTorsion(i))
	{
		return false;
	}

	float magnitude = fabs(_destination[i]);
	if (magnitude < _magnitudeThreshold)
	{
		return false;
	}

	return true;
}

int PlausibleRoute::countTorsions()
{
	int count = 0;
	for (size_t i = 0; i < dims(); i++)
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
	_activeTorsions = idxs;
	_paramPtrs.clear();
	_paramStarts.clear();
	
	for (size_t i = 0; i < idxs.size(); i++)
	{
		if (idxs[i] < 0)
		{
			continue;
		}

		WayPoints &wps = _wayPoints[idxs[i]];
		for (size_t j = 1; j < wps.size() - 1; j++)
		{
			WayPoint &wp = wps[j];
			
			_paramStarts.push_back(wp.fraction);
			_paramStarts.push_back(wp.progress);

			_paramPtrs.push_back(&wp.fraction);
			_paramPtrs.push_back(&wp.progress);
		}
	}
	
	std::vector<float> steps(_paramPtrs.size(), _stepSize * 2);
	setDimensionCount(_paramPtrs.size());
	setMaxJobsPerVertex(1);
	chooseStepSizes(steps);
}

bool PlausibleRoute::simplexCycle(std::vector<int> torsionIdxs)
{
	prepareAnglesForRefinement(torsionIdxs);
	if (_paramPtrs.size() == 0)
	{
		return false;
	}
	run();

	bool changed = false;

	float bs = bestScore();
	if (bs < _bestScore - 1e-6)
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

int PlausibleRoute::nudgeWaypoints()
{
	int changed = 0;
	startTicker("Nudging waypoints (" + f_to_str(_magnitudeThreshold, 1)
	            + " degrees +)");

	for (size_t i = 0; i < dims(); i++)
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
		
		std::vector<int> torsionIdxs;
		torsionIdxs.push_back(i);
		
		for (size_t j = i + 1; j < dims() && torsionIdxs.size() < 1; j++)
		{
			if (!validateMainTorsion(j))
			{
				continue;
			}
			
			torsionIdxs.push_back(j);
		}
		
		if (torsionIdxs.size() < 1)
		{
			continue;
		}
		
		bool result = simplexCycle(torsionIdxs);
		
		changed += (result ? 1 : 0);
	}

	postScore(_bestScore);
	finishTicker();

	return changed;
}

void PlausibleRoute::bringTorsionsToRange()
{
	for (size_t i = 0; i < dims(); i++)
	{
		while (_destination[i] >= 180)
		{
			_destination[i] -= 360;
		}
		while (_destination[i] < -180)
		{
			_destination[i] += 360;
		}
	}

}

void print(std::vector<bool> &flips)
{
	std::cout << flips.size() << ": ";
	for (size_t j = 0; j < flips.size(); j++)
	{
		std::cout << (flips[j] ? "Y" : "n");
	}

}

void addPermutation(std::vector<std::vector<bool> > &perms)
{
	std::vector<std::vector<bool> > orig = perms;
	std::vector<std::vector<bool> > new_set;
	
	for (size_t i = 0; i < orig.size(); i++)
	{
		for (size_t j = 0; j < 2; j++)
		{
			std::vector<bool> copy = orig[i];
			copy.push_back(j == 1 ? true : false);
			new_set.push_back(copy);
		}
	}
	
	perms = new_set;
}

std::vector<std::vector<bool> > permutations(int count)
{
	std::vector<std::vector<bool> > perms;
	perms.push_back(std::vector<bool>(1,false));
	perms.push_back(std::vector<bool>(1,true));

	for (int i = 0; i < count - 1; i++)
	{
		addPermutation(perms);
	}

	return perms;
}

void PlausibleRoute::setFlips(std::vector<int> &idxs, std::vector<bool> &fs)
{
	for (size_t j = 0; j < idxs.size(); j++)
	{
		_flips[idxs[j]] = fs[j];
	}
}

std::vector<int> PlausibleRoute::getTorsionSequence(int start, int max, 
                                                    bool validate, float maxMag)
{
	AtomGraph *g = grapher().graph(_torsions[start]);
	AtomGraph *orig = g;
	if (!g) return std::vector<int>();

	int count = 0;
	std::vector<int> idxs;
	idxs.push_back(start);
	
	while (count < max)
	{
		AtomGraph *next = nullptr;
		for (size_t j = 0; j < g->children.size(); j++)
		{
			Atom *a = g->children[j]->atom;
			AtomGraph *candidate = grapher().graph(a);
			int n = indexOfTorsion(candidate->torsion);
			if (n < 0)
			{
				continue;
			}

			if (candidate->torsion->coversMainChain())
			{
				next = candidate;
			}
			
			if (validate && !validateMainTorsion(n))
			{
				continue;
			}

			if (fabs(_destination[n]) < maxMag)
			{
				continue;
			}
			
			idxs.push_back(n);
			count++;
		}
		
		if (next == nullptr)
		{
			break;
		}
		
		g = next;
	}

	return idxs;
}


bool PlausibleRoute::flipTorsion(int idx)
{
	std::vector<int> idxs = getTorsionSequence(idx, 3, true, 30.f);
	
	if (idxs.size() == 0)
	{
		return false;
	}

	std::vector<bool> best(idxs.size(), false);
	for (size_t i = 0; i < idxs.size(); i++)
	{
		best[i] = _flips[idxs[i]];
	}

	std::vector<std::vector<bool> > putatives = permutations(idxs.size());
	bool changed = false;
	for (size_t i = 0; i < putatives.size(); i++)
	{
		setFlips(idxs, putatives[i]);

		float candidate = momentumScore(_nudgeCount);

		if (candidate < _bestScore - 1e-6)
		{
			_bestScore = candidate;
			best = putatives[i];
			changed = true;
		}
	}

	setFlips(idxs, best);
	postScore(_bestScore);
//	print(best);
//	std::cout << std::endl;
	
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
	_bestScore = momentumScore(_nudgeCount);
	
	bringTorsionsToRange();
	
	for (size_t i = 0; i < dims(); i++)
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

		float magnitude = fabs(_destination[i]);
		if (magnitude < 30.f)
		{
			continue;
		}
		
		flipTorsion(i);
	}
	
	clearMask();
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
	flipTorsionCycle(false);
}

void PlausibleRoute::nudgeWayPointCycles()
{
	int count = 0;
	_bestScore = momentumScore(_nudgeCount);
	
	while (true && count < _maximumCycles)
	{
		if (Route::_finish)
		{
			return;
		}
		
		int total = countTorsions();
		int change = nudgeWaypoints();
		/*
		std::cout << "Changed: " << change << " / " << total << ", "
		<< "score: " << _bestScore << std::endl;
		*/
		float frac = (float)change / (float)total;
		count++;

		if (_magnitudeThreshold < 40)
		{
			if (_nudgeCount < 12)
			{
				_nudgeCount = 12;
			}
			_stepSize = 0.01;
		}
		
		if (_bestScore < 0.6)
		{
			_minimumMagnitude = 2;
		}
		
		if (frac < 0.25 || total == 0)
		{
			_magnitudeThreshold /= 2;
		}
		
		if (!Route::_finish && 
		    _magnitudeThreshold < _minimumMagnitude && frac < 0.25)
		{
			_magnitudeThreshold = 90;
			break;
		}
	}
}

void PlausibleRoute::cycle()
{
	setTargets();
	flipTorsionCycles();

	if (Route::_finish)
	{
		return;
	}
	
	nudgeWayPointCycles();

	if (Route::_finish)
	{
		return;
	}

	flipTorsionCycle(false);
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

float PlausibleRoute::getTorsionAngle(int i)
{
	if (!_flips[i])
	{
		return _destination[i];
	}
	
	if (_destination[i] > 0)
	{
		return _destination[i] - 360;
	}
	else 
	{
		return _destination[i] + 360;
	}
}

float PlausibleRoute::getLinearInterpolatedTorsion(int i, float frac)
{
	float angle = getTorsionAngle(i);

	WayPoints wps = _wayPoints[i];
	WayPoint *start = nullptr;
	WayPoint *end = nullptr;

	for (size_t j = 1; j < wps.size(); j++)
	{
		start = &wps[j - 1];
		end = &wps[j];

		if (wps[j].fraction > frac)
		{
			break;
		}
	}

	float progress = start->progress;
	float proportion = ((frac - start->fraction) / 
	                    (end->fraction - start->fraction));

	float diff = (end->progress - start->progress) * proportion + progress;
	float angle_diff = angle * diff;

	return angle_diff;
}

void PlausibleRoute::addLinearInterpolatedPoint(float frac)
{
	Point point(dims(), 0);

	for (size_t i = 0; i < dims(); i++)
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

	float prop = 0;
	int mult = 1;
	float powered = 1;
	
	for (size_t i = 0; i < fit.size(); i++)
	{
		prop += fit[i] * powered;

		/* to make the next x^2, x^3 etc. */
		for (size_t i = 0; i < mult; i++)
		{
			powered *= frac;
		}
		mult++;
	}

	return angle * prop;
}

std::vector<PlausibleRoute::PolyFit> PlausibleRoute::polynomialFits()
{
	std::vector<PolyFit> fits;

	for (size_t i = 0; i < dims(); i++)
	{
		std::vector<float> xs, ys;
		int n = _wayPoints[i].size() - 1;

		for (int j = 0; j <= n; j++)
		{
			xs.push_back(_wayPoints[i][j].fraction);
			ys.push_back(_wayPoints[i][j].progress);
		}

		PolyFit pf = polyfit(xs, ys, n);
		
		fits.push_back(pf);
	}
	
	return fits;
}

void PlausibleRoute::addPolynomialInterpolatedPoint(std::vector<PolyFit> &fits,
                                                    float frac)
{
	Point point(dims(), 0);

	for (size_t i = 0; i < dims(); i++)
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
	switch (_type)
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

void PlausibleRoute::assignParameterValues(const SPoint &trial)
{
	assert(trial.size() == _paramPtrs.size());

	for (size_t i = 0; i < _paramPtrs.size(); i++)
	{
		float value = _paramStarts[i] + trial[i];
		*(_paramPtrs[i]) = value;
	}
}

bool PlausibleRoute::validateWayPoint(WayPoints &wps)
{
	float tolerance = 1 / (float)_nudgeCount;
	tolerance *= 2;

	for (size_t i = 0; i < wps.size() - 1; i++)
	{
		if (wps[i].progress > wps[i + 1].progress - tolerance)
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
		WayPoints &wps = _wayPoints[_activeTorsions[i]];
		if (!validateWayPoint(wps))
		{
			return false;
		}
	}

	return true;
}

int PlausibleRoute::sendJob(const SPoint &trial, bool force_update)
{
	assignParameterValues(trial);
	bool valid = validateWayPoints();
	float result = FLT_MAX;
	
	if (valid)
	{
		result = momentumScore(_nudgeCount);
	}
	
	_results[_jobNum] = result;
	_jobNum++;
	return (_jobNum - 1);
}

int PlausibleRoute::awaitResult(double *eval)
{
	if (_results.size() == 0)
	{
		return -1;
	}

	int val = _results.begin()->first;
	float result = _results.begin()->second;
	
	_results.erase(_results.begin());

	*eval = result;
	return val;
}

int PlausibleRoute::indexOfTorsion(BondTorsion *t)
{
	for (size_t i = 0; i < _torsions.size(); i++)
	{
		if (_torsions[i] == t)
		{
			return i;
		}
	}
	
	return -1;
}

void PlausibleRoute::extractWayPoints(PlausibleRoute *other)
{
	if (_calculators.size() == 0)
	{
		return;
	}

	const BondSequence *seq = _calculators[0]->sequence();
	const Grapher &g = seq->grapher();

	const BondSequence *otherseq = other->_calculators[0]->sequence();
	const Grapher &h = otherseq->grapher();

	for (size_t i = 0; i < g.graphCount(); i++)
	{
		AtomGraph *gr = g.graph(i);
		
		if (gr->torsion == nullptr)
		{
			continue;
		}

		for (size_t j = 0; j < h.graphCount(); j++)
		{
			AtomGraph *hr = h.graph(j);

			if (hr->torsion == nullptr || gr->torsion != hr->torsion)
			{
				continue;
			}
			
			if (gr->atom != hr->atom)
			{
				continue;
			}
			
			int src = other->indexOfTorsion(hr->torsion);
			int dest = indexOfTorsion(hr->torsion);
			
			if (src < 0 || dest < 0)
			{
				continue;
			}
			
			const WayPoints &wps = other->wayPoints(src);
			setWayPoints(dest, wps);
			bool flip = other->_flips[src];
			_flips[dest] = flip;
		}
	}
}

void PlausibleRoute::printWayPoints()
{
	std::cout << "Waypoints:" << std::endl;
	for (size_t i = 0; i < _wayPoints.size(); i++)
	{
		std::cout << _torsions[i]->desc() << " ";
		std::cout << " flip: " << (_flips[i] ? "yes" : "no") << "; ";
		for (size_t j = 0; j < _wayPoints[i].size(); j++)
		{
			std::cout << "(" << _wayPoints[i][j].progress << ", ";
			std::cout << _wayPoints[i][j].fraction << ") ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

const Grapher &PlausibleRoute::grapher() const
{
	const BondSequence *seq = _calculators[0]->sequence();
	const Grapher &g = seq->grapher();

	return g;
}

void PlausibleRoute::maskTails()
{
	const Grapher &g = grapher();
	const int quiet = 8;

	AtomGraph *gr = g.graph(0);
	/* add the ones in the forward direction */
	for (int i = 0; i < quiet; i++)
	{
		BondTorsion *bt = gr->torsion;
		int idx = indexOfTorsion(bt);
		if (idx < 0)
		{
			break;
		}

		_mask[idx] = false;
		gr = g.deepestChild(gr);
	}

	/* wind to the end */
	AtomGraph *next = gr;
	while (next != nullptr)
	{
		gr = next;
		next = g.deepestChild(gr);
	}
	
	/* add the ones in the backward direction */
	for (int i = 0; i < quiet; i++)
	{
		BondTorsion *bt = gr->torsion;
		int idx = indexOfTorsion(bt);
		if (idx < 0)
		{
			break;
		}

		_mask[idx] = false;
		gr = g.graph(gr->parent);
	}
}

void PlausibleRoute::prepareForAnalysis()
{
	float result = momentumScore(_nudgeCount);
	postScore(result);
	calculateProgression(200);
	
	for (size_t i = 0; i < pointCount(); i++)
	{
		submitJob(i, true, true);
	}
	
	retrieve();
	clearTickets();
}

