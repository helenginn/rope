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
#include "Icosahedron.h"

GuiRefls::GuiRefls()
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/reflections.fsh");

	_template = new Icosahedron();
	_template->setColour(0.5, 0.5, 0.5);
	_template->resize(0.3);
}

void GuiRefls::populateFromDiffraction(Diffraction *diffraction)
{
	size_t rfln = diffraction->reflectionCount();
	size_t vSize = _template->vertexCount();
	size_t iSize = _template->indexCount();
	_vertices.reserve(vSize * rfln);
	_indices.reserve(iSize * rfln);

	int limits[3] = {0, 0, 0};
	for (size_t i = 0; i < 3; i++)
	{
		limits[i] = diffraction->reciprocalLimitIndex(i);
	}

	int count = 0;

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

				glm::vec3 v = diffraction->reciprocal(i, j, k);
				v *= 50.;
				float heat = log(amp) / log(10);
				heat /= 6;
				const float exposure = 1.2; const float gamma = 1.5;
				float tmp = 1 - exp(-heat * heat * exposure);
				float tone = 3 * pow(tmp, 1.0 / gamma);
				float red = std::min(tone, 1.f);
				float green = std::min(tone, 2.f) - 1;
				float blue = std::min(tone, 3.f) - 2;

				_template->recolour(red, green, blue);
				_template->setPosition(v);
				_template->setExtra(glm::vec4(v, 1.));
				appendObject(_template);
				count++;
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
