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

#include "ThickLine.h"

ThickLine::ThickLine(bool proj) : Image("assets/images/mini_rope.png")
{
	setName("ThickLine");
	_renderType = GL_TRIANGLES;

	setUsesProjection(proj);
	_selectionResize = 1.0;

	if (proj)
	{
		setVertexShaderFile("assets/shaders/axes.vsh");
	}
	else
	{
		setVertexShaderFile("assets/shaders/unproj_axes.vsh");
		_searchType = Point;
	}

	setFragmentShaderFile("assets/shaders/axes.fsh");
}

void ThickLine::addThickLine(glm::vec3 start, glm::vec3 dir, glm::vec3 colour)
{
	{
		Snow::Vertex &v = addVertex(start);
		v.normal = dir;
		v.tex[0] = -0.5;
		v.tex[1] = 0;
		v.color = glm::vec4(colour, 1.0f);
	}

	{
		Snow::Vertex &v = addVertex(start + dir);
		v.normal = dir;
		v.tex[0] = -0.5;
		v.tex[1] = 1;
		v.color = glm::vec4(colour, 1.0f);
	}

	{
		Snow::Vertex &v = addVertex(start);
		v.normal = dir;
		v.tex[0] = +0.5;
		v.tex[1] = 0;
		v.color = glm::vec4(colour, 1.0f);
	}

	{
		Snow::Vertex &v = addVertex(start + dir);
		v.normal = dir;
		v.tex[0] = +0.5;
		v.tex[1] = 1;
		v.color = glm::vec4(colour, 1.0f);
	}

	addIndices(-4, -3, -2);
	addIndices(-3, -2, -1);
}

void ThickLine::addPoint(glm::vec3 p, glm::vec3 colour)
{
	glm::vec3 dir = (p - _last);

	if (!_assigned)
	{
		_assigned = true;
	}
	else 
	{
		addThickLine(_last, dir, colour);
	}

	_last = p;
}

void ThickLine::unMouseOver()
{
	if (isSelectable())
	{
		setHighlighted(false);
	}
}

bool ThickLine::mouseOver()
{
	if (isSelectable())
	{
		setHighlighted(true);
		return true;
	}

	return false;
}

void ThickLine::drag(double x, double y)
{
	if (_doJob)
	{
		_doJob(x, y);
	}
}
