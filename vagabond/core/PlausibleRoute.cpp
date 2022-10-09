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
		glm::vec3 diff = (s - d);
		atom->setOtherPosition("moving", diff);
		atom->setOtherPosition("target", s);
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

		if (getTorsionAngle(i) > 90.f && false)
		{
			wp.push_back(WayPoint(0.33, 0.33));
			wp.push_back(WayPoint(0.67, 0.67));
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
	for (size_t i = 0; i < steps; i++)
	{
		float rnd = rand() / (float)RAND_MAX;
		submitJob(i, rnd < 0.05);
	}
	
	retrieve();
	
	for (size_t i = 0; i < steps; i++)
	{
		float contribution = score(i);
		float frac = i / (float)pointCount();
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

bool PlausibleRoute::flipTorsions(bool main)
{
	startTicker("Flipping torsions");
	bool changed = false;
	_bestScore = momentumScore(_nudgeCount);

	for (size_t i = 0; i < dims(); i++)
	{
		clickTicker();
		
		if (!usingTorsion(i))
		{
			continue;
		}

		if (!validateMainTorsion(i))
		{
			continue;
		}

		float magnitude = fabs(_destination[i]);
		if (magnitude < 30.f)
		{
			continue;
		}
		
		_flips[i] = !_flips[i];
		float candidate = momentumScore(_nudgeCount);

		if ((_flipTorsions && candidate < _bestScore - 1e-6) ||
		    (!_flipTorsions && candidate < _bestScore * 0.6))
		{
			_bestScore = candidate;
			changed = true;
		}
		else
		{
			_flips[i] = !_flips[i];
		}
	}
	
	postScore(_bestScore);
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
		
		if (frac < 0.25 || total == 0)
		{
			_magnitudeThreshold /= 2;
		}
		
		if (_magnitudeThreshold < _minimumMagnitude && frac < 0.25)
		{
			_magnitudeThreshold = 90;
			break;
		}
	}
}

void PlausibleRoute::cycle()
{
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
	float step = 1 / (float)(steps - 1);
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
	float step = 1 / (float)(steps - 1);

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
	float result = momentumScore(16);
	postScore(result);
	calculateProgression(200);
}

