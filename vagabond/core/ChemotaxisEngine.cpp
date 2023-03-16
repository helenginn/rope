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

#include "ChemotaxisEngine.h"
#include <iostream>
#include <vagabond/utils/Hypersphere.h>

ChemotaxisEngine::ChemotaxisEngine(RunsEngine *ref) : Engine(ref)
{
	_hypersphere = new Hypersphere(n() - 1, n() * 10);
}

ChemotaxisEngine::~ChemotaxisEngine()
{
	delete _hypersphere;
	_hypersphere = nullptr;
}

void ChemotaxisEngine::run()
{
	_hypersphere->prepareFibonacci();
	
	int count = 0;
	while (count < _maxRuns)
	{
		cycle();
		count++;
	}

	currentScore();
}

bool ChemotaxisEngine::cycle()
{
	bool success = false;
	_original = _step;
	
	while (!success)
	{
		success = tumble();
		
		if (!success)
		{
			_step /= 2;
		}
		else
		{
			_step = _original;
		}
		
		if (_step < 0.01)
		{
			return false;
		}
	}

//	while (run())
	{

	}
	
	const std::vector<float> &curr = current();
	
	return true;
}

bool ChemotaxisEngine::tumble()
{
	float current = bestScore();
	setCurrent(bestResult());

	for (size_t i = 0; i < _hypersphere->count(); i++)
	{
		std::vector<float> pt = _hypersphere->scaled_point(i, _step);
		
		add_current_to(pt);
		sendJob(pt);
	}

	getResults();
	float new_score = bestScore();

	std::vector<float> chosen = bestResult();
	std::vector<float> diff = difference_from(chosen);
	
	bool better = (new_score < current);

	if (better)
	{
		setCurrent(chosen);
		currentScore();
		
		for (float &d : _direction)
		{
			d /= 100;
		}

		_direction = diff;
		
		return true;
	}
	
	return false;
}

bool ChemotaxisEngine::runner()
{
	float curr = bestScore();
	std::vector<float> pt = current();

	for (size_t i = 0; i < 10; i++)
	{
		add_to(pt, _direction);
		sendJob(pt);
	}

	getResults();
	float new_score = bestScore();
	std::vector<float> chosen = bestResult();
	
	bool better = (new_score < curr);

	if (better)
	{
		setCurrent(chosen);
		currentScore();
		return true;
	}
	
	return false;
}
