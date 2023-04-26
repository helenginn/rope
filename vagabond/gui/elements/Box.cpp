// Copyright (C) 2021 Helen Ginn

#include "Box.h"
#include "Text.h"
#include "Slider.h"
#include <iostream>

Box::Box() : Renderable()
{
	setFragmentShaderFile("assets/shaders/box.fsh");
	setVertexShaderFile("assets/shaders/box.vsh");

}

void Box::modify(Renderable *r)
{
	if (!_scroll)
	{
		return;
	}
	
	if (!dynamic_cast<Slider *>(r))
	{
		r->setFragmentShaderFile("assets/shaders/scrollbox.fsh");
		r->setScroll(true);
		
		Box *box = dynamic_cast<Box *>(r);
		if (box)
		{
			box->_bounds = _bounds;
		}
	}
}

void Box::setToScrollShaders()
{
	_scroll = true;
	setFragmentShaderFile("assets/shaders/scrollbox.fsh");

	for (size_t i = 0; i < objectCount(); i++)
	{
		Box *box = dynamic_cast<Box *>(object(i)); // blergh
		
		if (box)
		{
			box->setToScrollShaders();
		}
	}
}

glm::vec4 bounds2GL(glm::vec4 bounds)
{
	glm::vec4 forGL = glm::vec4(0.f);
	forGL[0] = 1 - 2 * bounds[0];
	forGL[1] = 2 * bounds[1] - 1;
	forGL[2] = 1 - 2 * bounds[2];
	forGL[3] = 2 * bounds[3] - 1;

	return forGL;
}

void Box::setScrollBox(glm::vec4 bounds)
{
	glm::vec4 forGL = bounds2GL(bounds);
	
	updateScrollBox(forGL);
	_bounds = bounds;
	
	assessBounds();
}

void Box::deleteSlider()
{
	removeObject(_slider);
	delete _slider;
	_slider = nullptr;
}

void Box::drawSlider()
{
	deleteSlider();

	Slider *slider = new Slider();
	slider->setDragResponder(_dragResponder);
	slider->setVertical(true);
	slider->resize(_bounds[2] - _bounds[0]);
	slider->setup("", 0, 0.99, 0.01);
	slider->setCentre(_bounds[3], (_bounds[2] + _bounds[0]) / 2);
	_slider = slider;
	addObject(slider);
}

void Box::assessBounds()
{
	float actual = _scrollBox[2] - _scrollBox[0];
	if (maximalHeight() * 2 > actual)
	{
		drawSlider();
	}
	else
	{
		deleteSlider();
	}
}

void Box::updateScrollBox(glm::vec4 bounds)
{
	_scrollBox = bounds;

	for (size_t i = 0; i < objectCount(); i++)
	{
		Box *box = dynamic_cast<Box *>(object(i)); // blergh
		
		if (box)
		{
			box->updateScrollBox(bounds);
		}
	}
}

void Box::updateScrollOffset(float new_offset)
{
	float diff = new_offset - _scrollOffset;
	
	for (Renderable *r : objects())
	{
		if (r != _slider)
		{
			r->addAlign(0, -diff);
			r->forceRender();
		}
	}

	_scrollOffset = new_offset;
}

void Box::copyScrolling(const Box *other)
{
	if (other->_scroll)
	{
		_scroll = other->_scroll;
		setToScrollShaders();
		updateScrollBox(other->_scrollBox);
	}
}

void Box::switchToScrolling(DragResponder *resp)
{
	setToScrollShaders();
	_dragResponder = resp;
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
	
	Renderable::unMouseOver();
}

void Box::extraUniforms()
{
	if (!_scroll)
	{
		return;
	}

	{
		GLuint u = glGetUniformLocation(_program, "box");
		glUniform4f(u, _scrollBox.x, _scrollBox.y, _scrollBox.z, _scrollBox.w);
		checkErrors("rebinding box");
	}
}
