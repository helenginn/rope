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

#include "Graph.h"
#include <vagabond/gui/elements/ThickLine.h>
#include <vagabond/gui/elements/Window.h>

Graph::Graph()
{
	clearVertices();

}

void Graph::addDataPoint(int series, float x, float y)
{
	_data[series].push_back({x, y});
}

void Graph::setRange(char axis, float min, float max)
{
	_axisRanges[axis] = {min, max};
}

void Graph::addAxes(float width, float height)
{
	auto load_thick_line = [this](ThickLine *tl, glm::vec3 &start,
	                          const glm::vec3 &move)
	{
		int divs = (glm::length(move) / 0.05) + 1;
		glm::vec3 dir = move / (float)divs;

		for (int i = 0; i < divs; i++)
		{
			glm::vec3 point = start + glm::vec3(0.5f, 0.5f, 0);
			tl->addPoint(point, {0.2, 0.2, 0.2});
			start += dir;
		}
		
		tl->setThickness(0.5 * Window::aspect());
		addObject(tl);
	};

	
	glm::vec3 start = {-width / 2.f, +height / 2.f, 0};
	glm::vec3 move_x = {width, 0.f, 0.f};
	glm::vec3 move_y = {0.f, -height, 0.f};

	glm::vec3 xstart = start;
	ThickLine *tlx = new ThickLine(false, "assets/images/vertical_line.png");
	load_thick_line(tlx, xstart, move_x);

	glm::vec3 ystart = start;
	ThickLine *tly = new ThickLine(false, "assets/images/vertical_line.png");
	load_thick_line(tly, ystart, move_y);
}

void Graph::setup(float width, float height)
{
	addAxes(width, height);
}
