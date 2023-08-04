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

SimplexEngine::SimplexEngine(RunsEngine *ref) : Engine(ref)
{
	_finish = false;
}

void SimplexEngine::reorderVertices()
{
	std::sort(_points.begin(), _points.end(), std::less<TestPoint &>());
	
}

void SimplexEngine::resetVertex(TestPoint &point)
{
	point.vertex.resize(n());
	for (size_t i = 0; i < point.vertex.size(); i++)
	{
		point.vertex[i] = 0;
	}
	point.eval = FLT_MAX;
	point.tickets.clear();
}

void SimplexEngine::allocateResources()
{
	_points.resize(n() + 1);

	for (size_t i = 0; i < n() + 1; i++)
	{
		resetVertex(_points[i]);
	}

	resetVertex(_centroid);
}

void SimplexEngine::findCentroid()
{
	resetVertex(_centroid);

	for (size_t j = 0; j < n(); j++)
	{
		for (size_t i = 0; i < n(); i++)
		{
			_centroid.vertex[i] += _points[j].vertex[i];
		}
	}

	for (size_t i = 0; i < n(); i++)
	{
		_centroid.vertex[i] /= (float)n();
	}
}

void SimplexEngine::printPoint(SPoint &point)
{
	for (size_t i = 0; i < point.size(); i++)
	{
		std::cout << point[i] << " ";
	}
}

void SimplexEngine::cycle()
{
	int count = 0;
	int shrink_count = 0;
	_changedParams = false;

	while (true)
	{
		if (count > _maxRuns || shrink_count >= 10 || _finish)
		{
			break;
		}

		reorderVertices();

		TestPoint &worst = _points[_points.size() - 1];
		float worst_score = worst.eval;

		TestPoint &second_worst = _points[_points.size() - 2];
		float second_worst_score = second_worst.eval;

		TestPoint &best = _points[0];
		float best_score = best.eval;
		
		Engine::clearResults();

		findCentroid();
		SPoint trial = scaleThrough(worst.vertex, _centroid.vertex, -1);
		sendJob(trial);

		float eval = FLT_MAX;
		getResults();
		Engine::findBestResult(&eval);
		clearResults();
		count++;
		
		if (eval > best_score && eval < second_worst_score)
		{
			worst.vertex = trial;
			worst.eval = eval;
			continue;
		}
		if (eval < best_score)
		{
			_changedParams = true;
			SPoint expanded = scaleThrough(trial, _centroid.vertex, -2);
			sendJob(expanded);
			
			float next = FLT_MAX;
			getResults();
			Engine::findBestResult(&next);
			clearResults();
			
			worst.vertex = (next < eval ? expanded : trial);
			worst.eval = (next < eval ? next : eval);
			continue;
		}
		else 
		{
			SPoint contracted;
			float compare;
			if (eval > second_worst_score && eval < worst_score)
			{
				contracted = scaleThrough(trial, _centroid.vertex, 0.5);
				compare = eval;
			}
			else 
			{
				contracted = scaleThrough(worst.vertex, _centroid.vertex, 0.5);
				compare = worst_score;
			}

			sendJob(contracted);

			float next = FLT_MAX;
			getResults();
			Engine::findBestResult(&next);
			clearResults();

			if (next < compare)
			{
				worst.vertex = contracted;
				worst.eval = next;
				continue;
			}
			else
			{
				shrink_count++;
				shrink();
			}
		}
	}
}

void SimplexEngine::collateResults()
{
	for (auto it = _scores.begin(); it != _scores.end(); it++)
	{
		int ticket = it->first;

		for (size_t i = 0; i < _points.size(); i++)
		{
			if (_points[i].tickets.count(ticket) > 0)
			{
				SPoint &p = _points[i].tickets[ticket];
				_points[i].eval = it->second.score;
				_points[i].vertex = it->second.vals;
			}
		}
	}

}

void SimplexEngine::pickUpResults()
{
	getResults();
	collateResults();
	clearResults();
}

void SimplexEngine::shrink()
{
	for (size_t i = 0; i < _points.size(); i++)
	{
		SPoint trial = scaleThrough(_points[i].vertex, _centroid.vertex, 0.8);
		int ticket = sendJob(trial);
		_points[i].tickets[ticket] = trial;
	}
	
	getResults();
	collateResults();
	clearResults();
}

void SimplexEngine::run()
{
	if (n() <= 0)
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
		_steps = std::vector<float>(n(), _step);
	}

	clearResults();

	std::vector<float> empty = std::vector<float>(n(), 0);
	sendJob(empty);
	
	getResults();
	float begin = FLT_MAX;
	findBestResult(&begin);

	allocateResources();
	sendStartingJobs();
	cycle();
	
	sendJob(bestResult());
	getResults();
}

bool SimplexEngine::classifyResults()
{
	getResults();

	bool changed = false;

	for (auto it = _scores.begin(); it != _scores.end(); it++)
	{
		int ticket = it->first;
		int eval = it->second.score;

		float worst = _points[_points.size() - 1].eval;
		float second_worst = _points[_points.size() - 2].eval;
		float best = _points[0].eval;

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

		float new_worst = _points[_points.size() - 1].eval;

		if (new_worst >= worst)
		{
			_points[_points.size() - 1].decision = ShouldContract;
		}

		if (changed)
		{
			findCentroid();
		}
	}
	
	clearResults();
	
	return changed;
}

void SimplexEngine::sendStartingJobs()
{
	for (size_t i = 0; i < _points.size(); i++)
	{
		SPoint trial;
		trial.resize(n());

		for (size_t j = 0; j < n(); j++)
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
	SPoint ret = SPoint(n(), 0.);

	for (size_t i = 0; i < n(); i++)
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

const SimplexEngine::SPoint &SimplexEngine::bestPoint() const
{
	return _points[0].vertex;
}

void SimplexEngine::finish()
{
	_finish = true;
}
