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

#include "RTAngles.h"
#include "TorsionBasis.h"
#include "../utils/svd/PCA.h"

class Instance;
class BondSequence;
class Residue;

class ConcertedBasis : public TorsionBasis
{
public:
	ConcertedBasis();
	~ConcertedBasis();

	virtual void prepare(int dims = 0);

	virtual Coord::Interpolate<float> valueForParameter(BondSequence *seq, 
	                                                    int tidx);
	
	void setCustom(bool custom)
	{
		_custom = custom;
	}
	
	bool fillFromInstanceList(Instance *instance, int axis,
	                          RTAngles angles);
protected:
	Coord::Interpolate<float> fullContribution(BondSequence *seq, int tidx);

	virtual float contributionForAxis(BondSequence *seq, int tidx, int axis, 
	                                  const Coord::Get &coord) const;
private:
	void setupAngleList();

	bool _custom = false;
	int _dims = 0;

	PCA::SVD _svd{};
};

#endif
