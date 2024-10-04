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
#include <set>
#include <map>
#include "ResidueTorsion.h"
#include <vagabond/utils/AcquireCoord.h>

class BondSequence;
class Parameter;
class Atom;

class TorsionBasis
{
public:
	TorsionBasis();
	virtual ~TorsionBasis() { };
	virtual void prepare();
	
	/** supply mask to switch refined torsion angles on/off 
	 * 	@param mask vector of ordered booleans, true=refined, false=masked,
	 * 		        corresponding to each torsion angle */
	virtual void supplyMask(std::vector<bool> mask) {}
	
	int addParameter(Parameter *param, Atom *atom = nullptr);

	virtual void absorbVector(const Coord::Get &coord);

	std::vector<int> grabIndices(const std::set<Parameter *> &params);
	void trimParametersToUsed(std::set<Parameter *> &params);

	const size_t parameterCount() const
	{
		return _params.size();
	}
	
	const std::vector<Parameter *> &parameters() const
	{
		return _params;
	}
	
	const float &referenceAngle(int i) const
	{
		return _angles[i].angle;
	}
	
	void setReferenceAngle(int i, float a)
	{
		_angles[i].angle = a;
	}
	
	Parameter *const parameter(int i) const
	{
		return _params[i];
	}
	
	int indexForParameter(Parameter *param);
protected:
	std::vector<Parameter *> _params;
	std::map<Parameter *, int> _param2Idx;
	std::vector<Atom *> _atoms;

	struct TorsionAngle
	{
		float angle;
		bool mask;
	};

	std::vector<TorsionAngle> _angles;

};

#endif
