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

#include "ElementSegment.h"
#include "AtomSegment.h"

AtomSegment::AtomSegment()
: CubicGrid<Density>(0, 0, 0)
, FFTCubicGrid<Density>(0, 0, 0)
{
	int nx = 0;
	int ny = 0;
	int nz = 0;

	this->Grid::setDimensions(nx, ny, nz);
}

void AtomSegment::addElementSegment(ElementSegment *seg)
{
	if (seg->nn() != nn())
	{
		throw std::runtime_error("Trying to add element segment "
		                         "of different nn()");
	}

	for (size_t i = 0; i < nn(); i++)
	{
		const float &r = seg->density(i, 0);
		const float &im = seg->density(i, 1);
		_data[i].value[0] += r * FIXED_MULTIPLY;
		_data[i].value[1] += im * FIXED_MULTIPLY;
	}
}

void AtomSegment::clear()
{
	for (size_t i = 0; i < nn(); i++)
	{
		Density &ele = element(i);
		ele.value[0] = 0;
		ele.value[1] = 0;
	}
}

float AtomSegment::density(int i, int j)
{
	Density &ele = element(i);
	float val = ele.value[j] / (float)FIXED_MULTIPLY;
	return val;
}
