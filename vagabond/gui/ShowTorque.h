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

#ifndef __vagabond__ShowTorque__
#define __vagabond__ShowTorque__

#include <vagabond/gui/elements/Image.h>
#include <vagabond/gui/elements/IndexResponder.h>
#include "ShowAbstractForce.h"

class Rod;
class Torque;
class Particle;

#define TORQUE_FORCE_MAGNITUDE (1.0f)

class ShowTorque : public Image, public ShowAbstractForce
{
public:
	ShowTorque(Particle *p, Rod *rod, Torque *torque, float magnitude, 
	           bool invert);
	
	virtual void update();
private:
	Rod *_rod{};
	Torque *_torque{};
	Particle *_particle{};
	float _magnitude{};
	bool _invert{};
};

#endif
