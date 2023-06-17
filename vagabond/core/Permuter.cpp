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

#include "Permuter.h"
#include <vagabond/utils/maths.h>

Permuter::Permuter(SpecificNetwork *sn, Mapped<float> *mapped,
                   std::atomic<bool> &stop, std::atomic<bool> &skip)
: AltersNetwork(sn, mapped, stop, skip)
{

}

void Permuter::permute(std::function<float()> score)
{
	std::vector<float> old_vals; 
	_getPoints(old_vals);
	float current = score();
	sendResponse("update_score", &current);
	std::vector<float> best = old_vals;

	std::vector<std::vector<int>> perms;
	int perm_length = std::min(5, (int)old_vals.size());
	perms = permutations(perm_length, 3);
	std::random_shuffle(perms.begin(), perms.end());

	int start = 0;
	while (true)
	{
		int end = start + perm_length;

		if (end > old_vals.size())
		{
			break;
		}

		for (const std::vector<int> &perm : perms)
		{
			std::vector<float> new_vals = best;

			int n = 0;
			auto lmb = [](const int &p) -> float
			{
				if (p == 0) return 0;
				if (p == 1) return -360;
				else return 360;
			};

			for (size_t i = start; i < end; i++)
			{
				float flipped = lmb(perm[n]) + old_vals[i];
				new_vals[i] = flipped;

				n++;
			}
			
			_setPoints(new_vals);

			float next = score();

			if (current > next)
			{
				std::cout << "IMPROVEMENT " << next << std::endl;
				current = next;
				sendResponse("update_score", &current);
				best = new_vals;
			}
			
			if (_skip)
			{
				_setPoints(best);
				_skip = false;
				return;
			}
			
			if (_stop)
			{
				_setPoints(best);
				throw 0;
			}
		}
		start++;
	}

	_setPoints(best);
}
