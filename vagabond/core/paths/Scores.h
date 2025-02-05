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

#ifndef __vagabond__Scores__
#define __vagabond__Scores__

#include "ResidueId.h"
#include "../engine/ElementTypes.h"
#include <mutex>

// helpful rant:
// The spaghetti code is really upsetting. I built this lovely control system which splits jobs arbitrarily on different threads, and somehow the control mechanism is just a giant IF statement maze now. I can split the scores up by residue ID but not by subunit. I’ve got a class called “ByResidueResult” and obviously I will need a “ByInstanceResult”, but I should make it templated for the future (“ResultBy<Custom>” where Custom can be whatever I like). In fact, “DetermineCustom” should be an actual function figuring out what “Custom” is. Then I can return e.g. instance ID or residue ID or whatever I like elsewhere in the code.

struct ActivationEnergy;

template <class Custom>
struct SingleResult
{
	Custom id{};
	float score = 0;
	float highest = 0;
};

template <class Custom>
struct ResultBy
{
	std::map<Custom, float> scores;
	std::map<Custom, float> activations;
	std::mutex *mutex = new std::mutex();
	int ticket;
	
	void destroy()
	{
		delete mutex;
		delete this;
	}

	void operator=(const SingleResult<Custom> &srr)
	{
		std::unique_lock<std::mutex> lock(*mutex);
		scores[srr.id] += srr.score;
		if (activations[srr.id] < srr.score)
		{
			activations[srr.id] = srr.score;
		}
	}
};

//typedef SingleResult<ResidueId> SingleResidueResult;
//typedef ResultBy<ResidueId> ByResidueResult;

#endif
