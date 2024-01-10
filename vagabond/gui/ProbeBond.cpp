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

ProbeBond::ProbeBond(const std::string &tag, const glm::vec3 &start,
                     const glm::vec3 &end)
: Image("assets/images/" + tag + ".png")
{
	setVertexShaderFile("assets/shaders/floating_box.vsh");
	setUsesProjection(true);

	glm::vec3 dir = end - start;
	glm::vec3 norm = {dir.y, -dir.x, 0};
	dir /= 5.f;
	norm /= 5.f;

	_vertices[0].pos = start - norm + dir;
	_vertices[1].pos = start + dir;
	_vertices[2].pos = end - norm - dir;
	_vertices[3].pos = end - dir;

}
