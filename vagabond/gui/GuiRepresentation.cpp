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

#include "GuiRepresentation.h"
#include <vagabond/core/AtomGroup.h>

void GuiRepresentation::watchAtomGroup(AtomGroup *ag)
{
	for (size_t i = 0; i < ag->size(); i++)
	{
		watchAtom((*ag)[i]);
	}
}

float control_points(glm::vec3 *a, glm::vec3 b, glm::vec3 c, glm::vec3 *d)
{
	glm::vec3 ca = c - *a;
	glm::vec3 bd = b - *d;
	ca *= 0.4;
	bd *= 0.4;
	*a = b;
	*d = c;
	float ok = 0;
	 
	if (ca.x == ca.x)
	{
		*a += ca;
	}
	else
	{
		ok = -1;
	}
	
	if (bd.x == bd.x)
	{
		*d += bd;
	}
	else
	{
		ok = +1;
	}
	
	return ok;
}

glm::vec3 bezier(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, double t)
{
	double c1 = (1 - t) * (1 - t) * (1 - t);
	double c2 = 3 * t * (1 - t) * (1 - t);
	double c3 = 3 * t * t * (1 - t);
	double c4 = t * t * t;

	p2 *= c1;
	p3 *= c4;
	p1 *= c2;
	p4 *= c3;

	glm::vec3 add = p1 + p2;	
	add += p3 + p4;
	
	return add;
}

std::vector<Snow::Vertex>
GuiRepresentation::bezierFrom(std::vector<Snow::Vertex> vs, int idx)
{
	int is[] = {idx - 1, idx + 0, idx + 1, idx + 2};
	if (is[0] < 0) is[0] = 0;
	if (is[2] >= vs.size()) is[2] = vs.size() - 1;
	if (is[3] >= vs.size()) is[3] = vs.size() - 1;

	glm::vec3 p1 = vs[is[0]].pos;
	glm::vec3 p2 = vs[is[1]].pos;
	glm::vec3 p3 = vs[is[2]].pos;
	glm::vec3 p4 = vs[is[3]].pos;

	std::vector<Snow::Vertex> next_set;
	next_set = GuiRepresentation::makeBezier(p1, p2, p3, p4);
	
	for (Snow::Vertex &v : next_set)
	{
		v.color = vs[idx].color;
	}

	return next_set;
}

std::vector<Snow::Vertex>
GuiRepresentation::makeBezier(glm::vec3 p1, glm::vec3 p2,
                              glm::vec3 p3, glm::vec3 p4,
                              bool overwrite)
{
	std::vector<Snow::Vertex> vs;

	float ok = control_points(&p1, p2, p3, &p4);

	float limit = 0.95;
	for (double t = 0; t <= limit; t += 1.0 / (float)POINTS_PER_BEZIER)
	{
		glm::vec3 p = bezier(p1, p2, p3, p4, t);
		Snow::Vertex v = new_vertex(p);

		if (ok < -0.5 && t < 0.05)
		{
			v.extra[0] = ok;
		}
		else if (ok > 0.5 && t > 0.75)
		{
			v.extra[0] = ok;
		}
		
		vs.push_back(v);
	}

	return vs;
}

std::vector<Snow::Vertex> 
GuiRepresentation::wireFrameToBezier(std::vector<Snow::Vertex> &vs,
                                     int buffer)
{
	std::vector<Snow::Vertex> bez;
	bez.reserve(vs.size() * POINTS_PER_BEZIER);

	for (size_t i = 1; i < vs.size() - buffer; i++)
	{
		std::vector<Snow::Vertex> next_set = bezierFrom(vs, i);
		bez.insert(bez.end(), next_set.begin(), next_set.end());
	}

	return bez;
}

Snow::Vertex GuiRepresentation::new_vertex(glm::vec3 p)
{
	Snow::Vertex v;
	memset(&v, 0, sizeof(Snow::Vertex));
	v.pos = p;
	v.color[3] = 1.;
	return v;
}

void GuiRepresentation::addCylinderIndices(std::vector<Snow::Vertex> &vertices,
                                           std::vector<GLuint> &indices, 
                                           size_t num)
{
	int begin = - num * 2;
	int half = num;

	for (size_t i = 0; i < num - 1; i++)
	{
		addIndex(indices, vertices, begin + 1);
		addIndex(indices, vertices, begin + 0);
		addIndex(indices, vertices, begin + half);
		addIndex(indices, vertices, begin + 1);
		addIndex(indices, vertices, begin + half);
		addIndex(indices, vertices, begin + half + 1);
		begin++;

	}

	begin = - num * 2;
	int first = num;
	int last = num - 1;
	addIndex(indices, vertices, begin + 0);
	addIndex(indices, vertices, begin + last);
	addIndex(indices, vertices, begin + last + first);
	addIndex(indices, vertices, begin + 0);
	addIndex(indices, vertices, begin + last + first);
	addIndex(indices, vertices, begin + first);
}
