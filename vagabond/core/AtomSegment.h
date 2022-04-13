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

#ifndef __vagabond__AtomSegment__
#define __vagabond__AtomSegment__

#include "FFTCubicGrid.h"
#include <atomic>

class ElementSegment;

/** \class AtomSegment
 *  class for atomically adding map values. Because float isn't intrinsically 
 *  supported in std:atomic, will have to go for fixed-point number. */

struct Density
{
	std::atomic<long> value[2];
};

class AtomSegment : public FFTCubicGrid<Density>
{
public:
	AtomSegment();
	
	template <class T>
	void getDimensionsFrom(const FFTCubicGrid<T> &other)
	{
		const int &nx = other.nx();
		const int &ny = other.ny();
		const int &nz = other.nz();
		
		setDimensions(nx, ny, nz, false);
		setOrigin(other.origin());
		setRealDim(other.realDim());
	}
	
	void addElementSegment(ElementSegment *seg);
	float density(int i, int j);
	
	void clear();

private:

};

#endif
