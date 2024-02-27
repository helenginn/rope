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

#include <iostream>
#include "MemoryTangle.h"
#include "Untangle.h"
#include <vagabond/core/Atom.h>
#include "TangledBond.h"

MemoryTangle::MemoryTangle(Untangle *untangle)
{
	_untangle = untangle;
}

bool MemoryTangle::incrementTangleTrial()
{
	if (_cancelled)
	{
		return true;
	}

	auto it = _tangles.end() - 1;

	int lead = 0;
	bool finished = false;
	while (true)
	{
		TangledState &last = *it;
		
		if (last.inLastConfiguration())
		{
			last.increment();
			
			if (it == _tangles.begin())
			{
				finished = true; break;
			}
			else
			{
				it--;
				lead++;
				
				if (lead >= _maxLead)
				{
					finished = true; break;
				}
			}
		}
		else
		{
			last.increment(); break;
		}
	}
	
	return finished;
	
	for (const TangledState &state : _tangles)
	{
		std::cout << state.option;
	}
}

void MemoryTangle::TangledState::setOption(int idx)
{
	StringString &strstr = options[idx];
	option = idx;
	std::string a = strstr.first;
	std::string b = strstr.second;
	
	memory->_untangle->switchConfs(atom, a, b, false);
	//		std::cout << atom->desc() << " switch from " << a << " to " << b << std::endl;
}

int MemoryTangle::TangledState::optionCount()
{
	return options.size();
}

void MemoryTangle::TangledState::increment()
{
	touch();
	setOption((option + 1) % optionCount()); // undo next job
}

void MemoryTangle::TangledState::touch()
{
	if (option >= 0)
	{
		setOption((option) % optionCount()); // undo previous job
	}
}

void MemoryTangle::tryTangle(Atom *atom)
{
	TangledState state(this, atom);
	_tangles.push_back(state);

	bool finished = false;
	float best_score = -1;
	std::vector<TangledState> best_states;
	
	while (!finished)
	{
		finished = incrementTangleTrial();
		
		float score = _untangle->biasedScore();
//		std::cout << " - " << score;
		if (best_score < score)
		{
			best_score = score;
			best_states = _tangles;
			_improved = _tangles.size();
			//std::cout << " !";
		}
		//std::cout << std::endl;
	}
	
	std::cout << "Chosen: " << best_score << std::endl;
	reinstate(best_states);
}

void MemoryTangle::retouch(std::vector<TangledState> &states)
{
	for (int i = states.size() - 1; i >= 0; i--)
	{
		states[i].touch();
	}
}

void MemoryTangle::reinstate(std::vector<TangledState> &states)
{
	retouch(_tangles);
	_tangles = states;
	retouch(_tangles);

	for (const TangledState &state : _tangles)
	{
		std::cout << state.option;
	}
	std::cout << std::endl;
	
}

int MemoryTangle::numberTanglesWithoutImprovement()
{
	return _tangles.size() - _improved;
}

OpSet<MemoryTangle::TangledState::StringString> 
MemoryTangle::TangledState::all_switches(Atom *atom)
{
	OpSet<std::string> confs = atom->conformerList();
	OpSet<StringString> pairs = confs.pairs_with(confs);
	pairs.filter([](const StringString &str) { return str.first < str.second; });
	
	return pairs;
}

MemoryTangle::TangledState::TangledState(MemoryTangle *parent, Atom *a)
{
	memory = parent;
	atom = a;
	option = -1;
	OpSet<StringString> opts = all_switches(atom);
	opts += {"A", "A"};
	
	options = opts.toVector();

}
