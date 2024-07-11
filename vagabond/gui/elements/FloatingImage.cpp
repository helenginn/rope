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

#include "FloatingImage.h"
#include "SnowGL.h"

FloatingImage::FloatingImage(std::string image, float mult, float yOff) 
: Image(image)
{
	setVertexShaderFile("assets/shaders/floating_box.vsh");
	setName("Floating image: " + image);
	makeQuad();
	correctBox(mult, yOff);
	setUsesProjection(true);
}

void FloatingImage::correctBox(float mult, float y_offset)
{
	glm::vec3 centre = centroid();

	for (Snow::Vertex &v : _vertices)
	{
		v.extra = glm::vec4(v.pos - centre, 0);
		v.extra *= mult;
		
		v.extra.y += y_offset;
		v.pos = centre;
	}
}

void FloatingImage::extraUniforms()
{
	glm::vec3 centre = glm::vec3(0.);
	if (_gl)
	{
		centre = _gl->getCentre();
	}

	GLuint uCentre = glGetUniformLocation(_program, "centre");
	glUniform3f(uCentre, centre[0], centre[1], centre[2]);
	Image::extraUniforms();
}
