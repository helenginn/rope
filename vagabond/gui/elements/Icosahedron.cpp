// abmap
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

#include "Icosahedron.h"
#include <iostream>

Icosahedron::Icosahedron() : SimplePolygon()
{
	makeIco();
	calculateNormals();

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/lighting.fsh");
}

void Icosahedron::makeIco()
{
	addVertex(2, 1, 0); // 0
	addVertex(-2, -1, 0); // 1
	addVertex(-2, 1, 0); // 2
	addVertex(2, -1, 0); // 3

	addVertex(0, 2, 1); // 4
	addVertex(0, -2, -1); // 5
	addVertex(0, -2, 1); // 6
	addVertex(0, 2, -1); // 7

	addVertex(1, 0, 2); // 8
	addVertex(-1, 0, -2); // 9
	addVertex(1, 0, -2); // 10
	addVertex(-1, 0, 2); // 11
	
	addIndices(6, 11, 8);
	addIndices(6, 1, 11);
	addIndices(6, 5, 1);
	addIndices(1, 9, 5);
	addIndices(1, 11, 2);
	addIndices(1, 2, 9);
	addIndices(3, 6, 8);
	addIndices(3, 5, 6);
	addIndices(3, 10, 5);
	addIndices(3, 0, 10);
	addIndices(3, 8, 0);
	addIndices(2, 7, 9);
	addIndices(2, 4, 7);
	addIndices(2, 11, 4);
	addIndices(10, 0, 7);
	addIndices(10, 7, 9);
	addIndices(10, 9, 5);
	addIndices(4, 8, 11);
	addIndices(4, 0, 8);
	addIndices(4, 0, 7);
	
	resize(1/sqrt(5));
}

void Icosahedron::triangulate()
{
	float before = envelopeRadius();
	glm::vec3 centre = centroid();
	
	Renderable::triangulate();
	
	for (size_t i = 0; i < vertexCount(); i++)
	{
		glm::vec3 &pos = _vertices[i].pos;
		pos -= centre;
		pos = before * glm::normalize(pos);
		pos += centre;
	}
	
	calculateNormals();
	rebufferVertexData();
}

void Icosahedron::calculateNormals()
{
	glm::vec3 centre = centroid();
	
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		glm::vec3 &pos1 = _vertices[_indices[i+0]].pos;
		glm::vec3 &pos2 = _vertices[_indices[i+1]].pos;
		glm::vec3 &pos3 = _vertices[_indices[i+2]].pos;

		glm::vec3 diff31 = pos3 - pos1;
		glm::vec3 diff21 = pos2 - pos1;

		glm::vec3 cross = glm::cross(diff31, diff21);
		glm::vec3 diff = pos1 - centre;
		
		double dot = glm::dot(diff, cross);
		if (dot < 0)
		{
			GLuint tmp = _indices[i];
			_indices[i] = _indices[i+1];
			_indices[i+1] = tmp;
		}
	}

	Renderable::calculateNormals();
}
