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

#ifndef __vagabond__TorsionBasis__
#define __vagabond__TorsionBasis__

#include <vector>

class BondTorsion;

class TorsionBasis
{
public:
	enum Type
	{
		TypeSimple,
		TypeConcerted,
	};

	TorsionBasis();
	virtual ~TorsionBasis() { };
	
	static TorsionBasis *newBasis(Type type);

	int addTorsion(BondTorsion *torsion);
	
	virtual float torsionForVector(int idx, const float *vec, int n) = 0;
	virtual void absorbVector(const float *vec, int n) = 0;
	
	const size_t torsionCount() const
	{
		return _torsions.size();
	}
	
	const BondTorsion *torsion(int i) const
	{
		return _torsions[i];
	}
protected:
	std::vector<BondTorsion *> _torsions;

};

#endif
