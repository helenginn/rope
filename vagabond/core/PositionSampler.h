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

#ifndef __vagabond__PositionSampler__
#define __vagabond__PositionSampler__

#include <vagabond/utils/Vec3s.h>

class PositionSampler
{
public:
	virtual ~PositionSampler() {};
	
	void setDoesBonds(bool bonds)
	{
		_doesBonds = bonds;
	}
	
	const bool &doesBonds() const
	{
		return _doesBonds;
	}
	
	void setDoesAtoms(bool atoms)
	{
		_doesAtoms = atoms;
	}
	
	const bool &doesAtoms() const
	{
		return _doesAtoms;
	}

	virtual bool prewarnAtoms(BondSequence *bc, const Coord::Get &get,
	                          Vec3s &positions) = 0;

	virtual glm::vec3 prewarnAtom(BondSequence *bc, const Coord::Get &get,
	                              int index)
	{
		Vec3s tmp;
		prewarnAtoms(bc, get, tmp);
		return tmp[index];
	}

	virtual float prewarnBond(BondSequence *bc, const Coord::Get &get,
	                              int index)
	{
		Floats tmp;
		prewarnBonds(bc, get, tmp);
		return tmp[index];
	}

	virtual void prewarnBonds(BondSequence *seq, const Coord::Get &get,
	                          Floats &torsions) = 0;
	
private:
	bool _doesAtoms = true;
	bool _doesBonds = true;
};

#endif
