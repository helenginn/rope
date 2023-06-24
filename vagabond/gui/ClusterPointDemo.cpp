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

#include "ClusterPointDemo.h"
#include "VagWindow.h"
#include <iostream>

ClusterPointDemo::ClusterPointDemo() : ClusterView()
{
	setUsesProjection(false);
	setSelectable(false);
	setVertexShaderFile("assets/shaders/box.vsh");
	setName("ClusterPointDemo");

	_size *= Window::ratio();
}

void ClusterPointDemo::makePoints()
{
	clearVertices();
	
	const double xspan = 1.6;
	double count = pointTypeCount();
	double xinit = 0.0 - xspan / 2;
	double xstep = xspan / count;
	xinit += xstep / 2;

	double yinit = 0.0;

	for (size_t i = 0; i < pointTypeCount(); i++)
	{
		double x = xinit + i * xstep;
		double y = yinit;

		glm::vec3 v = glm::vec3(x, y, 0);
		addPoint(v, i);
	}

}

