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

#include "matrix_functions.h"
#include "BestGuessTorsions.h"
#include "Route.h"

BestGuessTorsions::BestGuessTorsions(Route *const &route)
{
	_route = route;
}

float best_guess_torsion(Parameter *param)
{
	if (!param->isTorsion())
	{
		return 0;
	}

	glm::vec3 before[4]{};
	glm::vec3 after[4]{};

	for (size_t i = 0; i < param->atomCount(); i++)
	{
		before[i] = param->atom(i)->otherPosition("target");
		after[i] = param->atom(i)->otherPosition("moving");
	}

	float first = 0;
	float last = 0;
	for (float f = 0; f <= 1.01; f += 0.1)
	{
		glm::vec3 frac[4]{};
		for (int i = 0; i < 4; i++)
		{
			frac[i] = before[i] + after[i] * f;
		}
		
		float torsion = measure_bond_torsion(frac);
		if (f <= 0)
		{
			first = torsion;
			last = torsion;
		}
		else
		{
			while (torsion < last - 180.f) torsion += 360.f;
			while (torsion >= last + 180.f) torsion -= 360.f;
		}
		

		last = torsion;
	}

	return last - first;
}

void BestGuessTorsions::bestGuessTorsion(int idx)
{
	if (!_route->parameter(idx)->isTorsion())
	{
		return;
	}

	float best_guess = best_guess_torsion(_route->parameter(idx));
	_route->destination(idx) = best_guess;
}

void BestGuessTorsions::operator()()
{
	for (size_t i = 0; i < _route->motionCount(); i++)
	{
		if (fabs(_route->destination(i)) > 30)
		{
			bestGuessTorsion(i);
		}
	}
}

