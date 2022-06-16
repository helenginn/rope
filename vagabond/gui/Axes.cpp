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

#include <vagabond/core/MetadataGroup.h>
#include <vagabond/core/Molecule.h>
#include "Axes.h"

Axes::Axes(Cluster<MetadataGroup> *group, Molecule *m) : IndexResponder()
{
	setName("Axes");
	setImage("assets/images/arrow.png");
	setUsesProjection(true);
	_cluster = group;
	_molecule = m;
	_searchType = Point;
	
	_dirs.push_back(glm::vec3(1., 0., 0.));
	_dirs.push_back(glm::vec3(0., 1., 0.));
	_dirs.push_back(glm::vec3(0., 0., 1.));

	setVertexShaderFile("assets/shaders/axes.vsh");
	setFragmentShaderFile("assets/shaders/axes.fsh");
	
	prepareAxes();
	
#ifdef __EMSCRIPTEN__
	setSelectable(true);
#endif
}

bool Axes::mouseOver()
{
	int axis = currentVertex() / 4;
	interacted(axis, true);
	return (currentVertex() >= 0);
}

void Axes::unmouseOver()
{
	interacted(-1, true);
}

void Axes::click()
{
	int axis = currentVertex() / 4;
	interacted(axis, false);
}

void Axes::interacted(int idx, bool hover)
{
	if (!hover)
	{
		std::cout << "Axis: " << idx << std::endl;
	}
}

void Axes::reindex()
{
	size_t offset = indexOffset();

	for (size_t i = 0; i < _dirs.size(); i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			_vertices[i * 4 + j].extra[0] = i + 1 + offset;
		}
	}
}

size_t Axes::requestedIndices()
{
	return 3;
}

void Axes::prepareAxes()
{
	MetadataGroup *group = _cluster->dataGroup();
	int idx = group->indexOfObject(_molecule);
	glm::vec3 centre = _cluster->point(idx);

	for (size_t i = 0; i < _dirs.size(); i++)
	{
		glm::vec3 dir = _dirs[i];

		{
			Snow::Vertex &v = addVertex(centre);
			v.tex[0] = -0.5;
			v.tex[1] = 0;
			v.normal = dir;
		}

		{
			Snow::Vertex &v = addVertex(centre + dir);
			v.tex[0] = -0.5;
			v.tex[1] = 1;
			v.normal = dir;
		}

		{
			Snow::Vertex &v = addVertex(centre);
			v.tex[0] = +0.5;
			v.tex[1] = 0;
			v.normal = dir;
		}

		{
			Snow::Vertex &v = addVertex(centre + dir);
			v.tex[0] = +0.5;
			v.tex[1] = 1;
			v.normal = dir;
		}

		addIndices(-4, -3, -2);
		addIndices(-3, -2, -1);
	}
}
