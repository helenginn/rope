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
	_hypersphere = new Hypersphere(n() - 1, n() * 5);
}

ChemotaxisEngine::~ChemotaxisEngine()
{
	delete _hypersphere;
	_hypersphere = nullptr;
}

void ChemotaxisEngine::start()
{
	if (n() == 0)
	{
		return;
	}

	_hypersphere->prepareFibonacci();
	
	currentScore();
	std::cout << "Start: " << -bestScore() << std::endl;
	
	while (cycle())
	{

	}

	currentScore();
}

bool ChemotaxisEngine::cycle()
{
	bool success = false;
	
	while (!success)
	{
		success = tumble();
		
		if (!success)
		{
			_step /= 2;
		}
		
		if (_step < 0.01)
		{
			return false;
		}
	}

//	while (run())
	{

	}
	
	std::cout << "Now: ";
	const std::vector<float> &curr = current();
	
	for (size_t i = 0; i < curr.size(); i++)
	{
		std::cout << curr[i] << " ";
	}
	std::cout << std::endl;

	
	return true;
}

bool ChemotaxisEngine::tumble()
{
	for (size_t i = 0; i < _hypersphere->count(); i++)
	{
		std::vector<float> pt = _hypersphere->scaled_point(i, _step);
		add_current_to(pt);
		sendJob(pt);
	}

	getResults();
	
	float current = bestScore();

	float new_score = 0;
	std::vector<float> chosen = findBestResult(&new_score);
	std::vector<float> diff = difference_from(chosen);
	
	bool better = (new_score < current);

	std::cout << "After tumbling: " << -new_score << " (";
	std::cout << (better ? "improved" : "worse");
	std::cout << ")" << std::endl;

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

bool ChemotaxisEngine::run()
{
	std::vector<float> pt = current();
	for (size_t i = 0; i < 10; i++)
	{
		add_to(pt, _direction);
		sendJob(pt);
	}

	getResults();
	float new_score = 0;
	std::vector<float> chosen = findBestResult(&new_score);
	
	float current = bestScore();
	bool better = (new_score < current);

	std::cout << "After running: " << -new_score << " (";
	std::cout << (better ? "improved" : "worse");
	std::cout << ")" << std::endl;

	if (better)
	{
		setCurrent(chosen);
		currentScore();
		return true;
	}
	
	return false;
}
