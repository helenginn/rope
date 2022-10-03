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

PlausibleRoute::PlausibleRoute(Molecule *mol, Cluster<MetadataGroup> *cluster,
                               int dims)
: Route(mol, cluster, dims)
{

}

void PlausibleRoute::setup()
{
	if (_destination.size() == 0)
	{
		throw std::runtime_error("No destination set for route");
	}

	Route::setup();

	calculateLinearProgression(10);
}

void PlausibleRoute::setDestination(Point dest)
{
	_destination = dest;
	_wayPoints.clear();
	_flips.clear();
	
	for (size_t i = 0; i < _destination.size(); i++)
	{
		WayPoints wp;
		wp.push_back(WayPoint::startPoint());
		wp.push_back(WayPoint::midPoint());
		wp.push_back(WayPoint::endPoint());

		_wayPoints[i] = wp;
		_flips.push_back(false);
	}
}

float PlausibleRoute::momentumScore(int steps)
{
	calculateLinearProgression(steps);

	float cumulative = 0;
	for (size_t i = 0; i < steps; i++)
	{
		submitJob(i);
		cumulative += score();
	}
	
	return cumulative / (float)steps;
}

void PlausibleRoute::startTicker(std::string tag)
{
	HasResponder<Responder<Route>>::sendResponse("progress_" + tag, 
	                                             &_dims);

}

bool PlausibleRoute::nudgeWaypoints()
{
	float best = momentumScore(8);
	bool changed = false;
	std::cout << "Starting score: " << best << std::endl;
	startTicker("Nudging waypoints");

	for (size_t i = 0; i < _dims; i++)
	{
		clickTicker();

		if (!usingTorsion(i) || 
		    !_torsionLists[i].list[0].torsion.coversMainChain())
		{
			continue;
		}

		float magnitude = fabs(_destination[i]);
		if (magnitude < _magnitudeThreshold)
		{
			continue;
		}
		
		WayPoints &wp = _wayPoints[i];
		float start_prog = wp[1].progress;
		float progress = start_prog;
		float start_frac = wp[1].fraction;
		float fraction = start_frac;
		
		for (int i = -1; i <= 1; i += 2)
		{
			float step = (float)i * 0.1 + start_prog;
			wp[1].progress = step;
			float candidate = momentumScore(8);

			if (candidate < best)
			{
				best = candidate;
				progress = step;
				changed = true;
			}
		}
		
		/*
		for (int i = -1; i <= 1; i += 2)
		{
			float step = (float)i * 0.1 + start_frac;
			if (step < 0 || step > 1)
			{
				continue;
			}

			wp[1].fraction = step;
			float candidate = momentumScore(8);

			if (candidate < best)
			{
				std::cout << "New best fraction: " << best << std::endl;
				best = candidate;
				fraction = step;
				changed = true;
			}
		}
		*/
		
		wp[1].progress = progress;
		wp[1].fraction = fraction;
	}

	finishTicker();

	return changed;
}

bool PlausibleRoute::flipTorsions(bool main)
{
	startTicker("Flipping torsions");
	float best = momentumScore(4);
	bool changed = false;
	std::cout << "Starting score: " << best << std::endl;

	for (size_t i = 0; i < _dims; i++)
	{
		clickTicker();

		if (!usingTorsion(i) || (main &&
		    !_torsionLists[i].list[0].torsion.coversMainChain()))
		{
			continue;
		}

		float magnitude = fabs(_destination[i]);
		if (magnitude < 30.f)
		{
			continue;
		}
		
		_flips[i] = !_flips[i];
		float candidate = momentumScore(4);

		if (candidate < best)
		{
			std::cout << "New best: " << best << std::endl;
			best = candidate;
			changed = true;
		}
		else
		{
			_flips[i] = !_flips[i];
		}
	}

	finishTicker();

	return changed;
}

void PlausibleRoute::doCalculations()
{
	flipTorsions(true);
	while (flipTorsions())
	{

	}
	
//	flipTorsions();
	while (true)
	{
		bool change = nudgeWaypoints();

		if (!change)
		{
			_magnitudeThreshold /= 2;
			std::cout << "Magnitude threshold now: " << _magnitudeThreshold 
			<< std::endl;
		}
		
		if (_magnitudeThreshold < 10.f)
		{
			break;
		}
	}

	calculateLinearProgression(200);
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

void PlausibleRoute::addLinearInterpolatedPoint(float frac)
{
	Point point(_dims, 0);

	for (size_t i = 0; i < _dims; i++)
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

		point[i] = angle_diff;
	}
	
	addPoint(point);
}

void PlausibleRoute::calculateLinearProgression(int steps)
{
	clearPoints();
	float frac = 0;
	float step = 1 / (float)steps;

	for (size_t i = 0; i < steps + 1; i++)
	{
		addLinearInterpolatedPoint(frac);
		frac += step;
	}
}
