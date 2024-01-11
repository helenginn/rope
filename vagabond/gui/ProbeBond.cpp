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

#include "ProbeBond.h"

void ProbeBond::fixVertices(const glm::vec3 &start, const glm::vec3 &dir)
{
	clearVertices();

	{
		Snow::Vertex &v = addVertex(start);
		v.normal = dir;
		v.tex[0] = -0.5;
		v.tex[1] = 0;
	}

	{
		Snow::Vertex &v = addVertex(start + dir);
		v.normal = dir;
		v.tex[0] = -0.5;
		v.tex[1] = 1;
	}

	{
		Snow::Vertex &v = addVertex(start);
		v.normal = dir;
		v.tex[0] = +0.5;
		v.tex[1] = 0;
	}

	{
		Snow::Vertex &v = addVertex(start + dir);
		v.normal = dir;
		v.tex[0] = +0.5;
		v.tex[1] = 1;
	}

	addIndices(-4, -3, -2);
	addIndices(-3, -2, -1);

}

ProbeBond::ProbeBond(const std::string &tag, const glm::vec3 &start,
                     const glm::vec3 &end)
: Image("assets/images/" + tag + ".png")
{
//	setVertexShaderFile("assets/shaders/floating_box.vsh");
	setVertexShaderFile("assets/shaders/axes.vsh");
	setFragmentShaderFile("assets/shaders/axes.fsh");
	setUsesProjection(true);

	glm::vec3 dir = end - start;
	dir /= 4.f;

	fixVertices(start + dir, end - start - 2.f * dir);
}
