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
		float ratio = 2;
		float length = glm::length(dir);
		if (length < 2.f)
		{
			ratio = length / 2.f;
		}
		std::cout << "length: " << length << std::endl;

		addThickLine(start, dir);
		float transition = 0.87;

		for (int j = vertexCount() - 4; j < vertexCount(); j++)
		{
			Vertex v = vertex(j);
			v.normal /= ratio;
			if (j % 2 == 1)
			{
				v.tex[1] = transition;
			}
			setVertex(j, v);
		}

		start += dir;
		dir = glm::normalize(dir) * 0.3f;
		addThickLine(start, dir);

		for (int j = vertexCount() - 4; j < vertexCount(); j++)
		{
			Vertex v = vertex(j);
			v.normal = glm::normalize(v.normal) * 1.6f;
			if (j % 2 == 0)
			{
				v.tex[1] = transition;
			}
			setVertex(j, v);
		}

		if (_force->status() == AbstractForce::StatusCalculated)
		{
			setColour(0.52f, 0.1f, 0.67f);
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

