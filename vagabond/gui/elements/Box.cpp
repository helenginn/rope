// Copyright (C) 2021 Helen Ginn

#include "Box.h"
#include "Text.h"

Box::Box() : Renderable()
{
	setFragmentShaderFile("assets/shaders/box.fsh");
	setVertexShaderFile("assets/shaders/box.vsh");

}

void Box::addSplitQuad(double z)
{
	addVertex(-1, -1, z);
	addVertex(+1, -1, z);
	addVertex(-1, 0, z);
	addVertex(+1, 0, z);
	addVertex(-1, +1, z);
	addVertex(+1, +1, z);
	
	_vertices[0].tex[0] = 1;
	_vertices[0].tex[1] = 1;
	_vertices[1].tex[0] = 0;
	_vertices[1].tex[1] = 1;

	_vertices[2].tex[0] = 1;
	_vertices[2].tex[1] = 0.5;
	_vertices[3].tex[0] = 0;
	_vertices[3].tex[1] = 0.5;

	_vertices[4].tex[0] = 1;
	_vertices[4].tex[1] = 0;
	_vertices[5].tex[0] = 0;
	_vertices[5].tex[1] = 0;

	addIndices(0, 1, 2);
	addIndices(1, 2, 3);
	addIndices(2, 3, 4);
	addIndices(3, 4, 5);
}

void Box::addQuad(double z)
{
	addVertex(-1, -1, z);
	addVertex(-1, +1, z);
	addVertex(+1, -1, z);
	addVertex(+1, +1, z);
	
	_vertices[0].tex[0] = 0;
	_vertices[0].tex[1] = 1;
	_vertices[1].tex[0] = 0;
	_vertices[1].tex[1] = 0;
	_vertices[2].tex[0] = 1;
	_vertices[2].tex[1] = 1;
	_vertices[3].tex[0] = 1;
	_vertices[3].tex[1] = 0;

	addIndices(0, 1, 2);
	addIndices(1, 2, 3);

}

void Box::makeQuad()
{
	clearVertices();
	addQuad();
}

void Box::rescale(double x, double y)
{
	glm::mat3x3 mat = glm::mat3(1.);
	mat[0][0] = x;
	mat[1][1] = y;

	rotateRound(mat);
}

void Box::addAltTag(std::string text)
{
	Text *alt = new Text(text);
	alt->resize(0.5);
	setHover(alt);
}

bool Box::mouseOver()
{
	if (_hover)
	{
		_hover->setDisabled(false);
	}

	return false;
}

void Box::unMouseOver()
{
	if (_hover)
	{
		_hover->setDisabled(true);
	}
}
