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

#include "ScrollBox.h"
#include "Slider.h"

ScrollBox::ScrollBox() : Box()
{
	setVertexShaderFile("assets/shaders/box.vsh");
	setFragmentShaderFile("assets/shaders/scrollbox.fsh");

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

void ScrollBox::setBounds(glm::vec4 frac)
{
	glm::vec4 forGL = bounds2GL(frac);

	_screenBounds = frac;
	_glBounds = forGL;
	
	addSliderIfNeeded();
}

void ScrollBox::deleteSlider()
{
	deleteLater(_slider);
	_slider = nullptr;
}

void ScrollBox::drawSlider()
{
	deleteSlider();

	Slider *slider = new Slider();
	slider->setDragResponder(this);
	slider->setVertical(true);
	slider->resize(screenBoundHeight());
	slider->setup("", 0, 0.99, 0.01);
	slider->setCentre(screenBoundRight(), screenMidHeight());
	_slider = slider;
	addObject(slider);
}

void ScrollBox::addSliderIfNeeded()
{
	if (glMaxHeight() > glBoundHeight() && !hasSlider())
	{
		drawSlider();
	}
	else if (glMaxHeight() < glBoundHeight() && hasSlider())
	{
		deleteSlider();
	}

}

float ScrollBox::screenMidHeight()
{
	return (_screenBounds[2] + _screenBounds[0]) / 2;
}

float ScrollBox::screenBoundRight()
{
	return _screenBounds[3];
}

float ScrollBox::screenBoundHeight()
{
	return _screenBounds[2] - _screenBounds[0];
}

float ScrollBox::glBoundHeight()
{
	return _glBounds[0] - _glBounds[2];
}

float ScrollBox::glMaxHeight()
{
	return _content->maximalHeight();
}

void ScrollBox::extraUniforms()
{
	GLuint u = glGetUniformLocation(_program, "box");
	glUniform4f(u, _glBounds[0], _glBounds[1], _glBounds[2], _glBounds[3]);
}

void ScrollBox::heightChanged()
{

}

void ScrollBox::setContent(Box *box)
{
	box->setResponder(this);
	_content = box;
	_content->setVertexShaderFile("assets/shaders/box.vsh");
	_content->setFragmentShaderFile("assets/shaders/scrollbox.fsh");
	_content->setDelegate(this);
	addObject(_content);
}

void ScrollBox::respond()
{
	heightChanged();
}

void ScrollBox::updateScrollOffset(float offset)
{
	float diff = _scrollOffset - offset;
	
	_content->addAlign(0, diff / 2);
	_content->forceRender(true, false);

	_scrollOffset = offset;

}

void ScrollBox::finishedDragging(std::string tag, double x, double y)
{
	float hx = x * glMaxHeight();
	updateScrollOffset(hx);
}
