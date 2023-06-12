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

#ifndef __vagabond__AreaMeasurer__
#define __vagabond__AreaMeasurer__

#include "AtomPosMap.h"
#include "vagabond/core/Fibonacci.h"

class SurfaceAreaHandler;
class ContactSheet;
struct Job;

class AreaMeasurer
{
public:
	AreaMeasurer(SurfaceAreaHandler *handler, int n_points = 200);
	~AreaMeasurer();
	
	void setJob(Job *j)
	{
		_job = j;
	}
	
	Job *job() const
	{
		return _job;
	}

	void copyAtomMap(AtomPosMap &posMap)
	{
		_posMap = posMap;
	}
	
	const AtomPosMap &atomPosMap() const
	{
		return _posMap;
	}
	
	/** calculates surface area of previously copied atom map in Angstroms. */
	float surfaceArea();
	/** calculates exposure of atom, i.e. the percentage of points not in the overlap with the other atoms in posmap */
	float fibExposureSingleAtom(Atom *atom);

private:
	Job *_job = nullptr;
	AtomPosMap _posMap;
	Fibonacci _lattice;

	SurfaceAreaHandler *_handler = nullptr;
	ContactSheet *_contacts = nullptr;
};

/** calculates the exposed area from atom's lattice exposure and  vdw radius*/
float areaFromExposure(float exposure, Atom *atom);

/** get VdWRadius from an atom*/
float getVdWRadius(Atom *atom);

#endif
