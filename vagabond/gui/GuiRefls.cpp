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

#include <iostream>
#include <vagabond/core/Diffraction.h>
#include "GuiRefls.h"
#include "VagWindow.h"
#include <vagabond/utils/maths.h>
#include <vagabond/gui/elements/Icosahedron.h>

GuiRefls::GuiRefls() : SimplePolygon()
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/simple_point.vsh");
	setFragmentShaderFile("assets/shaders/reflections.fsh");
	setImage("assets/images/blob.png");
	_renderType = GL_POINTS;
	setName("Gui refls");

	_size *= Window::ratio();
}

void GuiRefls::populateFromDiffraction(Diffraction *diffraction)
{
	size_t rfln = diffraction->reflectionCount();
	_vertices.reserve(rfln);
	_indices.reserve(rfln);

	int limits[3] = {0, 0, 0};
	for (size_t i = 0; i < 3; i++)
	{
		limits[i] = diffraction->reciprocalLimitIndex(i);
	}

	
	float ave = diffraction->mean();
	std::cout << "average: " << ave << std::endl;

	for (int k = -limits[2]; k < limits[2]; k++)
	{
		for (int j = -limits[1]; j < limits[1]; j++)
		{
			for (int i = -limits[0]; i < limits[0]; i++)
			{
				float amp = diffraction->element(i, j, k).amplitude();
				
				if (amp != amp)
				{
					continue;
				}
				
				amp /= ave;

				glm::vec3 v = diffraction->reciprocal(i, j, k);
				v *= 10;
				float heat = log(amp);

				const float exposure = 1.0; const float gamma = 2.0;
				float tmp = 1 - exp(-heat * heat * exposure);
				float tone = 3 * pow(tmp, 1.0 / gamma) - 1;
				
				float red = 0; float green = 0; float blue = 0;
				val_to_cluster4x_colour(tone, &red, &green, &blue);
				red /= 255.f; green /= 255.f; blue /= 255.f;

				Vertex &vert = addVertex(v);
				vert.color = {red, green, blue, 1.f};
				vert.extra = glm::vec4(v, 1.f);
				addIndex(-1);
			}
		}
	}
}

void GuiRefls::render(SnowGL *gl)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	Renderable::render(gl);

	glDisable(GL_DEPTH_TEST);
}

void GuiRefls::extraUniforms()
{
	GLuint uSlice = glGetUniformLocation(_program, "slice");
	glUniform1f(uSlice, _slice);

	const char *uniform_name = "size";
	GLuint u = glGetUniformLocation(_program, uniform_name);
	glUniform1f(u, _size);
}
