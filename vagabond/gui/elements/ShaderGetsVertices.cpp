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

#include "ShaderGetsVertices.h"
#include "Renderable.h"
#include "Library.h"

ShaderGetsVertices::ShaderGetsVertices(Renderable *parent) : ShaderGets(parent)
{

}

ShaderGetsVertices::~ShaderGetsVertices()
{
	for (auto it = _bVertices.begin(); it != _bVertices.end(); it++)
	{
		GLuint bV = it->second;
		glDeleteBuffers(1, &bV);
	}

	for (auto it = _bElements.begin(); it != _bElements.end(); it++)
	{
		GLuint bE = it->second;
		glDeleteBuffers(1, &bE);
	}
}

void ShaderGetsVertices::setupVBOBuffers()
{
	prepareBuffers();
	
	GLuint program = _parent->program();

	if (program == 0 || vPointer() == nullptr || iPointer() == nullptr)
	{
		return;
	}

	int vao = _parent->vaoForContext();
	
	if (vao == 0)
	{
		return;
	}

	glBindVertexArray(vao);

	GLuint bv = 0;
	glGenBuffers(1, &bv);
	_bVertices[program] = bv;

	glBindBuffer(GL_ARRAY_BUFFER, bv);
	checkErrors("binding array buffer");
	
	setupExtraVariables();
	enablePointers();

	checkErrors("binding attributes");

	glBufferData(GL_ARRAY_BUFFER, vSize(), vPointer(), GL_STATIC_DRAW);
	checkErrors("vbo buffering");

	// indices
	GLuint be = 0;
	glGenBuffers(1, &be);
	_bElements[program] = be;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bElements[program]);
	checkErrors("index array binding");
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize(), iPointer(), GL_STATIC_DRAW);
	checkErrors("index array buffering");

	glBindVertexArray(0);

}

void ShaderGetsVertices::prepareBuffers()
{
	_vertexCopy = _parent->vertices();
}

void ShaderGetsVertices::extraVariables()
{
	GLuint program = _parent->program();

	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "normal");
	glBindAttribLocation(program, 2, "color");
	glBindAttribLocation(program, 3, "extra");

	if (_parent->hasTexture())
	{
		glBindAttribLocation(program, 4, "tex");
	}
}

void ShaderGetsVertices::setupExtraVariables()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	if (_parent->hasTexture())
	{
		glEnableVertexAttribArray(4); 
	}
}

void ShaderGetsVertices::enablePointers()
{
	/* Vertices */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
	                      (void *)(0 * sizeof(float)));
	checkErrors("binding vertices");

	/* Normals */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void *)(3 * sizeof(float)));
	checkErrors("binding indices");

	/* Colours */
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
	                      (void *)(6 * sizeof(float)));

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (void *)(10 * sizeof(float)));

	if (_parent->hasTexture())
	{
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
		                      (void*)(14 * sizeof(float)));

		checkErrors("rebinding texture attributes");
	}
}

size_t ShaderGetsVertices::vSize()
{
	return sizeof(Vertex) * _parent->vertexCount();
}

void *ShaderGetsVertices::vPointer()
{
	return (void *)&_parent->vertices()[0];
}

size_t ShaderGetsVertices::iSize()
{
	return sizeof(GLuint) * _parent->indexCount();
}

void *ShaderGetsVertices::iPointer()
{
	return _parent->iPointer();
}


void ShaderGetsVertices::rebufferIndexData()
{
	GLuint program = _parent->program();

	// don't rebuffer if you haven't set up the buffers, will crash
	if (_bElements.count(program) == 0)
	{
		return;
	}

	GLuint be = _bElements[program];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, be);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize(), iPointer(), GL_STATIC_DRAW);
	checkErrors("index array rebuffering");
}

void ShaderGetsVertices::rebufferVertexData()
{
	GLuint program = _parent->program();
	if (_bVertices.count(program) == 0)
	{
		return;
	}
	
	prepareBuffers();
	
	GLuint bv = _bVertices[program];
	glBindBuffer(GL_ARRAY_BUFFER, bv);

	glBufferData(GL_ARRAY_BUFFER, vSize(), vPointer(), GL_STATIC_DRAW);
	checkErrors("vertex array rebuffering");
}
