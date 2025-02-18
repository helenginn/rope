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

#include <vagabond/core/forces/Rod.h>
#include <vagabond/core/forces/Torque.h>
#include <vagabond/core/forces/Particle.h>
#include "ShowTorque.h"

ShowTorque::ShowTorque(Particle *p, Rod *rod, Torque *torque, float magnitude)
: Image("assets/images/half_arrow.png")
{
	_rod = rod;
	_torque = torque;
	_particle = p;
	_magnitude = magnitude;

	Image::setVertexShaderFile("assets/shaders/axes.vsh");
	Image::setFragmentShaderFile("assets/shaders/axes.fsh");
	Image::setUsesProjection(true);
	
	update();
}

void ShowTorque::update()
{
	Image::clearVertices();

	if (_torque->status() != Torque::StatusUnknown &&
	    _torque->status() != Torque::StatusIgnored)
	{
		setDisabled(false);
		glm::vec3 middle = _rod->midpoint();
		glm::vec3 centre = _particle->pos();
		glm::vec3 offset = middle - centre; 
		middle = centre + offset * 0.6f;

		glm::vec3 dir = cross(offset, _torque->moment());
		dir *= _magnitude * TORQUE_FORCE_MAGNITUDE;
		std::cout << "mag: " << _magnitude << std::endl;

		glm::vec3 start = middle - dir / 2.f;

		addThickLine(start, dir, _magnitude > 0);

		if (_torque->status() == AbstractForce::StatusCalculatedDirOnly)
		{
			setColour(0.6f, 0.2f, 1.0f);
		}
		else if (_torque->status() == AbstractForce::StatusCalculated)
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

