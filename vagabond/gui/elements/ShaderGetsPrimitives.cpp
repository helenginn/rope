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

#include "ShaderGetsPrimitives.h"
#include "Renderable.h"
#include "Library.h"

ShaderGetsPrimitives::ShaderGetsPrimitives(Renderable *r) : ShaderGetsVertices(r)
{

}

void ShaderGetsPrimitives::prepareBuffers()
{
	_primitives.clear();
	_triIndices.clear();
	std::vector<Vertex> vs = _parent->vertices();
	std::vector<GLuint> is = _parent->indices();
	
	for (int i = 0; i < is.size(); i+= 3)
	{
		for (int j = 0; j < 3; j++)
		{
			int k = (j + 1) % 3;
			int l = (k + 1) % 3;
			
			int i_j = is[i + j];
			int i_k = is[i + k];
			int i_l = is[i + l];

			Primitive p;
			p.pos = vs[i_j].pos;
			p.second = vs[i_k].pos;
			p.third = vs[i_l].pos;
			p.normal = vs[i_j].normal;
			p.color = vs[i_j].color;
			_primitives.push_back(p);
			
			_triIndices.push_back(i + j);
		}

	}
}

void ShaderGetsPrimitives::extraVariables()
{
	GLuint program = _parent->program();

	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "second");
	glBindAttribLocation(program, 2, "third");
	glBindAttribLocation(program, 3, "normal");
	glBindAttribLocation(program, 4, "color");
}

void ShaderGetsPrimitives::setupExtraVariables()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
}

void ShaderGetsPrimitives::enablePointers()
{
	/* Vertices */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Primitive), 
	                      (void *)(0 * sizeof(float)));
	checkErrors("binding vertices");

	/* Second and third vertices */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Primitive),
	                      (void *)(3 * sizeof(float)));
	checkErrors("binding second");

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Primitive), 
	                      (void *)(6 * sizeof(float)));
	checkErrors("binding third");

	/* Normals */
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Primitive),
	                      (void *)(9 * sizeof(float)));
	checkErrors("binding normals");

	/* Colours */
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Primitive), 
	                      (void *)(12 * sizeof(float)));
	checkErrors("binding colors");
}

size_t ShaderGetsPrimitives::vSize()
{
	return sizeof(Primitive) * _primitives.size();
}

void *ShaderGetsPrimitives::vPointer()
{
	return (void *)&_primitives[0];
}

size_t ShaderGetsPrimitives::iSize()
{
	return sizeof(GLuint) * _triIndices.size();
}

void *ShaderGetsPrimitives::iPointer()
{
	return (void *)(&_triIndices[0]);
}
