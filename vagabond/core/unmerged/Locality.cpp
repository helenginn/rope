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

#include "Locality.h"

void Locality::operator+=(const Obs &obs)
{
	(*this)[obs.hkl] = obs.info;
}

float Locality::sum() const
{
	float sum = 0;
	for (auto it = this->begin(); it != this->end(); it++)
	{
		sum += it->second.intensity;
	}

	return sum;
}

float Locality::weight(const Reflection::HKL &hkl) const
{
	if (hkl == _obs.hkl)
	{
		return 1;
	}
	float sqdist = sq_dist_from(Obs{hkl, {}}, _obs);

	return 1 / sqdist;
}
