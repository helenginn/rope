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

#ifndef __vagabond__MechanicalBasis__
#define __vagabond__MechanicalBasis__

#include "TorsionBasis.h"
#include "engine/ForceField.h"
#include <vagabond/utils/svd/PCA.h>
#include "Job.h"

struct Result;
class ForceField;

class MechanicalBasis : public TorsionBasis
{
public:
	MechanicalBasis();

	/* matrix describing connectivity between atoms */
	void supplyDistances(PCA::Matrix &m, std::vector<Atom *> atoms);

	void supplyForceField(ForceField *ff)
	{
		_forceField = ff;
	}

	virtual float parameterForVector(BondSequence *seq, 
	                                 int tidx, const AcquireCoord &coordinate, 
	                                 int n);
	virtual void prepare(int dims = 0);
	virtual void prepareRecalculation();

	bool doesTorsionAffectRestraint(const ForceField::Restraint &r, 
	                                const Parameter *t);

	void refreshMechanics(AtomPosMap &aps);
private:
	void incrementTime();
	void checkForceField(AtomPosMap &aps);
	void assignStartingVelocities();

	bool _run = false;
	
	struct MechAngle
	{
		float start;
		float change;
		float velocity;
		float acceleration;
		bool mask;
	};

	std::vector<MechAngle> _mechAngles;
	PCA::Matrix _distances;
	std::vector<Atom *> _atoms;
	std::map<Atom *, int> _atomIdxs;
	ForceField *_forceField = nullptr;
};

#endif
