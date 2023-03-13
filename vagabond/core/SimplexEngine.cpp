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

void SimplexEngine::printPoint(SPoint &point)
{
	for (size_t i = 0; i < point.size(); i++)
	{
		std::cout << point[i] << " ";
	}
}

void SimplexEngine::singleCycle()
{
	int count = 0;
	int shrink_count = 0;
	_changedParams = false;
	std::cout << "Total dimensions: " << n() << std::endl;

	while (true)
	{
		if (count > _maxJobRuns || shrink_count >= 10 || _finish)
		{
			break;
		}

		reorderVertices();

		TestPoint &worst = _points[_points.size() - 1];
		double worst_score = worst.eval;

		TestPoint &second_worst = _points[_points.size() - 2];
		double second_worst_score = worst.eval;

		TestPoint &best = _points[0];
		double best_score = best.eval;
		
		std::cout << std::endl;
		for (TestPoint &tp : _points)
		{
			std::cout << tp << std::endl;
		}

		findCentroid();
		SPoint trial = scaleThrough(worst.vertex, _centroid.vertex, -1);
		sendJob(trial);

		double eval = FLT_MAX;
		awaitResult(&eval);
		std::cout << "new point ";
		printPoint(trial);
		std::cout << " evaluate: " << eval << std::endl;
		count++;
		
		if (eval > best_score && eval < second_worst_score)
		{
			worst.vertex = trial;
			worst.eval = eval;
			continue;
		}
		if (eval < best_score)
		{
			std::cout << "option two, try expand" << std::endl;
			_changedParams = true;
			SPoint expanded = scaleThrough(trial, _centroid.vertex, -2);
			sendJob(expanded);
			
			double next = FLT_MAX;
			awaitResult(&next);
			
			std::cout << "next: " << next << std::endl;
			std::cout << (next < eval ? "yes expand" : "just trial") << std::endl;
			worst.vertex = (next < eval ? expanded : trial);
			worst.eval = (next < eval ? next : eval);
			continue;
		}
		else 
		{
			SPoint contracted;
			double compare;
			if (eval > second_worst_score && eval < worst_score)
			{
				std::cout << "option three and a half" << std::endl;
				contracted = scaleThrough(trial, _centroid.vertex, 0.5);
				compare = eval;
			}
			else 
			{
				std::cout << "option three and three quarters" << std::endl;
				contracted = scaleThrough(worst.vertex, _centroid.vertex, 0.5);
				compare = worst_score;
			}

			sendJob(contracted);

			double next = FLT_MAX;
			awaitResult(&next);
			
			if (next < compare)
			{
				std::cout << "option three and seven eighths" << std::endl;
				worst.vertex = contracted;
				worst.eval = next;
				continue;
			}
			else
			{
				std::cout << "option SHRINK!" << std::endl;
				shrink_count++;
				shrink();
			}
		}
	}
}

void SimplexEngine::pickUpResults()
{
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
				SPoint &p = _points[i].tickets[ticket];
				_points[i].eval = eval;
				_points[i].vertex = p;
			}
		}
	}
}

void SimplexEngine::shrink()
{
	for (size_t i = 0; i < _points.size(); i++)
	{
		SPoint trial = scaleThrough(_points[i].vertex, _centroid.vertex, 0.8);
		int ticket = sendJob(trial);
		_points[i].tickets[ticket] = trial;
	}

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
				SPoint &p = _points[i].tickets[ticket];
				_points[i].vertex = p;
			}
		}
	}
}

bool SimplexEngine::run()
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

	if (_steps.size() == 0)
	{
		throw std::runtime_error("No step sizes chosen for SimplexEngine");
	}

	std::vector<float> empty = std::vector<float>(_dims, 0);
	sendJob(empty);
	
	double begin = FLT_MAX;
	awaitResult(&begin);

	allocateResources();
	sendStartingJobs();
	singleCycle();
	
	sendJob(bestPoint());
	double end = FLT_MAX;
	awaitResult(&end);
	
	return (end < begin);
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

int SimplexEngine::sendJob(const SPoint &trial, bool force_update)
{
	// to be implemented downstream
	return -1;
}

void SimplexEngine::sendStartingJobs()
{
	for (size_t i = 0; i < _points.size(); i++)
	{
		SPoint trial;
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
	}
	
	pickUpResults();
}

SimplexEngine::SPoint SimplexEngine::scaleThrough(SPoint &p, SPoint &q, float k)
{
	SPoint ret = SPoint(_dims, 0.);

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
		SPoint trial = scaleThrough(_points[i].vertex, 
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

		SPoint trial = scaleThrough(last.vertex, _centroid.vertex, -1.5);
		int ticket = sendJob(trial);
		last.tickets[ticket] = trial;
	}
}

void SimplexEngine::sendShrinkJobs()
{
	for (size_t i = 0; i < _points.size(); i++)
	{
		SPoint trial = scaleThrough(_points[i].vertex, _centroid.vertex, 0.8);
		int ticket = sendJob(trial);
		_points[i].tickets[ticket] = trial;

		_points[i].decision = ShouldReflect;
	}
}

void SimplexEngine::sendContractionJob(int i)
{
	SPoint trial = scaleThrough(_points[i].vertex, 
	                           _centroid.vertex, 0.8);
	int ticket = sendJob(trial);
	_points[i].tickets[ticket] = trial;
}

void SimplexEngine::sendExpansionJob(int i)
{
	SPoint trial = scaleThrough(_points[i].vertex, 
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

const SimplexEngine::SPoint &SimplexEngine::bestPoint() const
{
	return _points[0].vertex;
}

void SimplexEngine::finish()
{
	_finish = true;
}
