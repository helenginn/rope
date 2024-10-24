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

#ifndef __vagabond__FFTCubicGrid__
#define __vagabond__FFTCubicGrid__

#include <fftw3.h>
#include "FFT.h"
#include "CubicGrid.h"

template <class T>
class FFTCubicGrid : virtual public CubicGrid<T>, virtual public FFT<T>
{
public:
	FFTCubicGrid() : CubicGrid<T>(0, 0, 0), FFT<T>() {}
	FFTCubicGrid(int nx, int ny, int nz) : 
	CubicGrid<T>(nx, ny, nz),
	FFT<T>() {};

private:

};

#endif
