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

Engine::Engine(RunsEngine *ref)
{
	_ref = ref;
	_n = ref->parameterCount();
}

void Engine::currentScore()
{
	if (_current.size() == 0)
	{
		_current = std::vector<float>(_n, 0);
	}

	clearResults();
	sendJob(_current);
	getResults();
	findBestResult(&_best);
	clearResults();
}

std::vector<float> Engine::findBestResult(float *score)
{
	auto it = _scores.begin();
	TicketScore *ts = nullptr;
	float best = FLT_MAX;
	
	while (it != _scores.end())
	{
		if (it->second.received && it->second.score < best)
		{
			best = it->second.score;
			ts = &(it->second);
		}

		it++;
	}

	if (ts == nullptr)
	{
		*score = FLT_MAX;
		return std::vector<float>();
	}
	
	*score = best;
	return ts->vals;
}

void Engine::getResults()
{
	int job_id = -1;

	do
	{
		float score = _ref->getResult(&job_id);
		_scores[job_id].score = score;
		_scores[job_id].received = true;
	}
	while (job_id >= 0);
}

void Engine::sendJob(std::vector<float> &all)
{
	int ticket = _ref->sendJob(all);
	TicketScore ts;
	ts.vals = all;

	_scores[ticket] = ts;
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