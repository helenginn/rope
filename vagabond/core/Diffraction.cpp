// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "Diffraction.h"

Diffraction::Diffraction(int nx, int ny, int nz) 
: Grid<VoxelDiffraction>(nx, ny, nz),
TransformedGrid<VoxelDiffraction>(nx, ny, nz)
{

}

Diffraction::Diffraction(RefList &list) 
: Grid<VoxelDiffraction>(0, 0, 0),
TransformedGrid<VoxelDiffraction>(0, 0, 0)
{
	_list = &list;

	Reflection::HKL hkl = list.maxSymHKL();
	
	for (size_t i = 0; i < 3; i++)
	{
		hkl[i] = 2 * hkl[i] + 1;
	}

	this->setDimensions(hkl[0], hkl[1], hkl[2]);
	setRealMatrix(_list->frac2Real());
	
	populateReflections();
}

void Diffraction::populateReflections()
{
	for (size_t i = 0; i < nn(); i++)
	{
		element(i).setAmplitudePhase(NAN, NAN);
	}

	for (size_t i = 0; i < _list->reflectionCount(); i++)
	{
		_list->addReflectionToGrid(this, i);
	}

}

size_t Diffraction::reflectionCount()
{
	return _list->reflectionCount() * _list->symOpCount() * 2;
}
