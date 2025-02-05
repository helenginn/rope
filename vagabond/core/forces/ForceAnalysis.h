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

#ifndef __vagabond__ForceAnalysis__
#define __vagabond__ForceAnalysis__

#include <map>
#include <vagabond/utils/OpSet.h>
#include "StaticForces.h"
#include "AbstractForce.h"

class AbstractForce;
class BondLength;
class AtomContent;
class Particle;
class Torque;
class Force;
class Atom;
class Rod;

typedef std::function<void(Particle *p, Rod *rod, 
                           Torque *torque, float magnitude)> 
HandleTorque;
typedef std::function<void(Particle *p, Force *force, float magnitude)> 
HandleForce;

class ForceAnalysis
{
public:
	ForceAnalysis(AtomContent *group);

	void setHandleForce(const HandleForce &func)
	{
		_handleForce = func;
	}

	void setHandleTorque(const HandleTorque &func)
	{
		_handleTorque = func;
	}

	void toggleReason(AbstractForce::Reason reason, bool include);

	void convert();
	
	void calculateUnknown();
	void resetUnknowns();
private:
	void createRods();
	void createParticles();
	void createTorsionTorques();
	void createBondAngleTorques();
	void createCloseContacts();
	void createElectrostaticContacts();
	void augmentSymmetry();

	void applyTorque(Particle *p, Rod *rod, Torque *torque, float magnitude);
	void applyForce(Particle *p, Force *force, float magnitude);

	AtomContent *_group{};
	StaticForces _forces;

	std::map<Atom *, Particle *> _atom2Particle;
	std::map<BondLength *, Rod *> _bond2Rod;
	
	HandleTorque _handleTorque{};
	HandleForce _handleForce{};
	
	OpSet<AbstractForce *> _allForces;
};

#endif
