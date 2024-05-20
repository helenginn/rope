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

#include "Engine.h"
#include <iostream>
#include <cstring>

Engine::Engine(RunsEngine *ref)
{
	_ref = ref;
	_ref->resetTickets();
	_n = _ref->parameterCount(this);
}

void Engine::currentScore()
{
	if (_bestResult.size() == 0)
	{
		_bestResult = std::vector<float>(_n, 0);
	}

	clearResults();
	sendJob(_bestResult);
	getResults();
	_currentScore = findBestScore();
	clearResults();
}

float Engine::findBestScore()
{
	float best = FLT_MAX;

	std::unique_lock<std::mutex> lock(_mutex);
	for (auto it = _scores.begin(); it != _scores.end(); it++)
	{
		if (it->second.received && it->second.score < best)
		{
			best = it->second.score;
		}
	}
	
	return best;
}

bool Engine::getOneResult()
{
	int job_id = -1;
	float score = _ref->getResult(&job_id, this);

	if (job_id < 0)
	{
		return false;
	}

	std::unique_lock<std::mutex> lock(_mutex);
	_scores[job_id].score = score;
	_scores[job_id].received = true;

	if (_verbose)
	{
		if (score >= _bestScore)
		{
			std::cout << "." << std::flush;
		}
		else
		{
			std::cout << "+" << std::flush;
		}
	}

	if (score < _bestScore)
	{
		_bestScore = score;
		_bestResult = _scores[job_id].vals;
	}
	return true;
}

void Engine::getResults()
{
	while (true)
	{
		if (!getOneResult())
		{
			return;
		}
	}
}

int Engine::sendJob(const std::vector<float> &all)
{
	int ticket = _ref->sendJob(all, this);
	TicketScore ts{};
	ts.vals = all;

	std::unique_lock<std::mutex> lock(_mutex);
	_scores[ticket] = ts;
	return ticket;
}

std::vector<float> Engine::difference_from(std::vector<float> &other)
{
	std::vector<float> ret = _current;

	for (size_t i = 0; i < ret.size(); i++)
	{
		ret[i] = other[i] - _current[i];
	}
	
	return ret;
}

void Engine::add_to(std::vector<float> &other, const std::vector<float> &add)
{
	for (size_t i = 0; i < other.size(); i++)
	{
		other[i] += add[i];
	}
}

void Engine::add_current_to(std::vector<float> &other)
{
	add_to(other, _current);
}

void Engine::preRun()
{
	_n = _ref->parameterCount(this);
	_current.clear();
	_bestResult.clear();
	_bestScore = FLT_MAX;

	if (n() == 0)
	{
		std::cout << "No parameters" << std::endl;
		return;
	}

	currentScore();
	_startScore = _currentScore;
	if (_verbose)
	{
		std::cout << _currentScore << " -> " << std::flush;
	}
}

void Engine::postRun()
{
	currentScore();
	_endScore = _currentScore;
	if (_verbose)
	{
		std::cout << " -> " << _endScore << " (";

		for (const float &f : bestResult())
		{
			std::cout << f << ", ";
		}
		std::cout << ")" << std::endl;
	}

	_improved = (_endScore < _startScore - 1e-6);
}

void Engine::start()
{
	preRun();
	run();
	postRun();
}

void Engine::trueGradients(float *g, const float *x)
{

}

void Engine::estimateGradients(float *g, const float *x)
{
	std::vector<float> vals(n());
	memcpy(&vals[0], x, n() * sizeof(float));

	int *high_tickets = new int[n()];
	int *low_tickets = new int[n()];
	float interval = (_step / 500);
	
	for (size_t i = 0; i < n(); i++)
	{
		float orig = vals[i];
		vals[i] = orig - interval / 2;
		low_tickets[i] = sendJob(vals);
		vals[i] = orig + interval / 2;
		high_tickets[i] = sendJob(vals);
		vals[i] = orig;
	}
	
	getResults();

	for (size_t i = 0; i < n(); i++)
	{
		float upper = _scores[high_tickets[i]].score;
		float lower = _scores[low_tickets[i]].score;
		float diff = (upper - lower) / interval;
		g[i] = diff;
	}
	
	delete [] high_tickets;
	delete [] low_tickets;
}

void Engine::grabGradients(float *g, const float *x)
{
	if (_ref->returnsGradients())
	{
		trueGradients(g, x);
	}

	estimateGradients(g, x);
}
