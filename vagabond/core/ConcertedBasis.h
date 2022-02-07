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

#ifndef __vagabond__ConcertedBasis__
#define __vagabond__ConcertedBasis__

#include "TorsionBasis.h"

class ConcertedBasis : public TorsionBasis
{
public:
	ConcertedBasis();

	virtual float torsionForVector(int idx, const float *vec, int n) { return 0; };
	virtual void absorbVector(const float *vec, int n) { };
	virtual void prepare();

	/** only the torsions available in the mask will be used for calculating
	 *  principle axes of motion */
	virtual void supplyMask(std::vector<bool> mask);
	
	size_t activeBonds();
private:
	void setupAngleList();
	std::vector<bool> _refineMask;
	size_t _nActive;

};

#endif
