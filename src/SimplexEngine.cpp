// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "SimplexEngine.h"
#include <float.h>
#include <algorithm>
#include <iostream>

SimplexEngine::SimplexEngine()
{
	_finish = false;
}

void SimplexEngine::reorderVertices()
{
	std::sort(_points.begin(), _points.end(), std::less<TestPoint &>());
	
}

void SimplexEngine::resetVertex(TestPoint &point)
{
	point.vertex.resize(_dims);
	for (size_t i = 0; i < point.vertex.size(); i++)
	{
		point.vertex[i] = 0;
	}
	point.eval = FLT_MAX;
	point.tickets.clear();
}

void SimplexEngine::allocateResources()
{
	_points.resize(_dims + 1);

	for (size_t i = 0; i < _dims + 1; i++)
	{
		resetVertex(_points[i]);
	}

	resetVertex(_centroid);
}

void SimplexEngine::findCentroid()
{
	resetVertex(_centroid);

	for (size_t j = 0; j < _dims; j++)
	{
		for (size_t i = 0; i < _dims; i++)
		{
			_centroid.vertex[i] += _points[j].vertex[i];
		}
	}

	for (size_t i = 0; i < _dims; i++)
	{
		_centroid.vertex[i] /= (float)_dims;
	}
}

void SimplexEngine::printPoint(Point &point)
{
	for (size_t i = 0; i < point.size(); i++)
	{
		std::cout << point[i] << " ";
	}

	std::cout << std::endl;
}

void SimplexEngine::cycle()
{
	int count = 0;
	int shrink_count = 0;
	
	while (true)
	{
		bool changed = awaitResults();
		count++;
		
		if (count == 1)
		{
			for (size_t i = 0; i < _points.size(); i++)
			{
				_points[i].decision = ShouldReflect;
			}
		}
		
		if (count >= 300 || shrink_count >= 3 || _finish)
		{
			break;
		}
		
		if (changed)
		{
			sendDecidedJobs();
			shrink_count = 0;
		}
		else
		{
			sendShrinkJobs();
			shrink_count++;
		}
	}

}

void SimplexEngine::run()
{
	if (_dims <= 0)
	{
		throw std::runtime_error("Nonsensical dimensions for SimplexEngine");
	}

	if (_maxJobs <= 0)
	{
		throw std::runtime_error("Nonsensical maximum job count per vertex"
		                         "for SimplexEngine");
	}

	if (_steps == nullptr)
	{
		throw std::runtime_error("No step sizes chosen for SimplexEngine");
	}

	allocateResources();
	sendStartingJobs();
	cycle();
}

bool SimplexEngine::awaitResults()
{
	bool changed = false;

	double worst = _points[_points.size() - 1].eval;
	double second_worst = _points[_points.size() - 2].eval;
	double best = _points[0].eval;

	while (true)
	{
		double eval = FLT_MAX;
		int ticket = awaitResult(&eval);
		
		if (ticket < 0)
		{
			break;
		}
		

		for (size_t i = 0; i < _points.size(); i++)
		{
			if (_points[i].tickets.count(ticket) > 0)
			{
				Decision job = _points[i].decision;

				/* no improvement on 2nd worst */
				if (eval > worst)
				{
					_points[i].decision = ShouldReflect;
				}
				else if (eval > second_worst)
				{
					_points[i].decision = ShouldReflect;
				}
				else if (eval < second_worst)
				{
					changed = true;

					TestPoint &w = _points[_points.size() - 1];
					w.vertex = _points[i].tickets[ticket];
					w.eval = eval;
					
					if (eval < best)
					{
						w.decision = ShouldExpand;
					}
					else 
					{
						w.decision = ShouldReflect;
					}
				}

				reorderVertices();
				_points[i].tickets.erase(ticket);
				break;
			}
		}
	}


	double new_worst = _points[_points.size() - 1].eval;
	
	if (new_worst >= worst)
	{
		_points[_points.size() - 1].decision = ShouldContract;
	}
	
	if (changed)
	{
		findCentroid();
	}
	
	return changed;
}

int SimplexEngine::sendJob(const Point &trial)
{
	// to be implemented downstream
	return -1;
}

void SimplexEngine::sendStartingJobs()
{
	for (size_t i = 0; i < _points.size(); i++)
	{
		Point trial;
		trial.resize(_dims);

		for (size_t j = 0; j < _dims; j++)
		{
			float val = (i == j) ? _steps[i] : 0;
			trial[j] = val;
		}
		
		int ticket = sendJob(trial);
		
		if (ticket < 0)
		{
			throw(std::runtime_error("::sendJob has not been implemented "
			                              " in SimplexEngine superclass."));
		}
		
		_points[i].tickets[ticket] = trial;
		_points[i].decision = ShouldReflect;
	}
}

SimplexEngine::Point SimplexEngine::scaleThrough(Point &p, Point &q, float k)
{
	Point ret = Point(_dims, 0.);

	for (size_t i = 0; i < _dims; i++)
	{
		ret[i] = q[i] + k * (p[i] - q[i]);
	}
	
	return ret;
}

void SimplexEngine::sendReflectionJob(int i)
{
	int num = 0;
	for (size_t j = 0; j < i; j++)
	{
		Point trial = scaleThrough(_points[i].vertex, 
		                           _points[j].vertex, -1);
		int ticket = sendJob(trial);
		_points[i].tickets[ticket] = trial;
		num++;

		if (num >= _maxJobs)
		{
			break;
		}
	}
	
	if (i == 0)
	{
		TestPoint &last = _points.back();

		Point trial = scaleThrough(last.vertex, _centroid.vertex, -1.5);
		int ticket = sendJob(trial);
		last.tickets[ticket] = trial;
	}
}

void SimplexEngine::sendShrinkJobs()
{
	for (size_t i = 0; i < _points.size(); i++)
	{
		Point trial = scaleThrough(_points[i].vertex, _centroid.vertex, 0.8);
		int ticket = sendJob(trial);
		_points[i].tickets[ticket] = trial;

		_points[i].decision = ShouldReflect;
	}
}

void SimplexEngine::sendContractionJob(int i)
{
	Point trial = scaleThrough(_points[i].vertex, 
	                           _centroid.vertex, 0.8);
	int ticket = sendJob(trial);
	_points[i].tickets[ticket] = trial;
}

void SimplexEngine::sendExpansionJob(int i)
{
	Point trial = scaleThrough(_points[i].vertex, 
	                           _centroid.vertex, 1.5);
	int ticket = sendJob(trial);
	_points[i].tickets[ticket] = trial;
}

void SimplexEngine::sendDecidedJobs()
{
	for (size_t i = 0; i < _points.size(); i++)
	{
		if (_points[i].decision == ShouldExpand)
		{
			sendExpansionJob(i);
		} 
		else if (_points[i].decision == ShouldReflect)
		{
			sendReflectionJob(i);
		}
		else if (_points[i].decision == ShouldContract)
		{
			sendContractionJob(i);
		}

		_points[i].decision = ShouldReflect;
	}

}

int SimplexEngine::awaitResult(double *eval)
{
	return -1;
}

const SimplexEngine::Point &SimplexEngine::bestPoint() const
{
	return _points[0].vertex;
}

