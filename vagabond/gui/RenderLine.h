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

#ifndef __vagabond__RenderLine__
#define __vagabond__RenderLine__

#include <vector>
#include <vagabond/gui/elements/Vertex.h>
#include <vagabond/gui/elements/Renderable.h>

std::vector<Vertex> 
vertices_for_vector(const glm::vec3 &start, const glm::vec3 &end)
{
	std::vector<Vertex> vs(4);
	glm::vec3 dir = end - start;

	vs[0].pos = start;
	vs[0].normal = dir;
	vs[0].tex[0] = -0.5;
	vs[0].tex[1] = 0;

	vs[1].pos = start + dir;
	vs[1].normal = dir;
	vs[1].tex[0] = -0.5;
	vs[1].tex[1] = 1;

	vs[2].pos = start;
	vs[2].normal = dir;
	vs[2].tex[0] = +0.5;
	vs[2].tex[1] = 0;

	vs[3].pos = start + dir;
	vs[3].normal = dir;
	vs[3].tex[0] = +0.5;
	vs[3].tex[1] = 1;
	
	return vs;
}

void add_indices_for_renderable(Renderable *renderable)
{
	renderable->addIndices(-4, -3, -2);
	renderable->addIndices(-3, -2, -1);
}

#endif
