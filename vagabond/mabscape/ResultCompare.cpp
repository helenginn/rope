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

#include "ResultCompare.h"
#include <map>

float compareResults(const PosToComp::Result &left,
                     const PosToComp::Result &right,
                     float radius)
{
	std::map<std::string, const Positions::Placement *> lookup;
	for (const Positions::Placement &pl : right.placements)
	{
		lookup[pl.name] = &pl;
	}

	float sum = 0;
	int count = 0;

	for (const Positions::Placement &pl : left.placements)
	{
		if (!lookup.count(pl.name) || pl.copies.empty())
		{
			continue;
		}

		const Positions::Placement &other = *lookup.at(pl.name);

		float nearest = FLT_MAX;
		for (const glm::vec3 &copy : other.copies)
		{
			nearest = std::min(nearest, glm::length(copy - pl.copies[0]));
		}

		if (nearest < FLT_MAX)
		{
			sum += PosToComp::closeness(nearest, radius);
			count++;
		}
	}

	return (count ? sum / count : 0.f);
}
