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

#include "ShowForce.h"
#include "ShowTorque.h"
#include <vagabond/core/forces/Force.h>
#include <vagabond/core/forces/Particle.h>

ShowForce::ShowForce(Particle *p, Force *force, float magnitude)
: Image("assets/images/axis.png")
{
	_particle = p;
	_force = force;
	_magnitude = magnitude;

	Image::setVertexShaderFile("assets/shaders/axes.vsh");
	Image::setFragmentShaderFile("assets/shaders/axes.fsh");
	Image::setUsesProjection(true);
	
	update();
}

void ShowForce::update()
{
	Image::clearVertices();

	if (_force->status() != AbstractForce::StatusUnknown
	    && _force->status() != AbstractForce::StatusIgnored)
	{
		setDisabled(false);
		glm::vec3 start = _particle->pos();
		glm::vec3 dir = _force->get_vector();
		dir *= _magnitude * TORQUE_FORCE_MAGNITUDE;

		addThickLine(start, dir);
		
		if (_force->status() == AbstractForce::StatusCalculated)
		{
			setColour(0.2f, 0.2f, 1.0f);
		}
		else
		{
			setColour(0.0f, 0.0f, 0.0f);
		}
	}
	else
	{
		setDisabled(true);
	}

	forceRender();
}

