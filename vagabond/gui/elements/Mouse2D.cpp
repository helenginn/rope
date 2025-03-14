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

#include "Window.h"
#include "Mouse2D.h"
#include "SelectionBox.h"
#include <SDL2/SDL.h>

Mouse2D::Mouse2D(Scene *prev) : Scene(prev)
{
	_alwaysOn = true;
	_2D = true;
}

Mouse2D::~Mouse2D()
{

}

void Mouse2D::interpretMouseButton(SDL_MouseButtonEvent button, bool dir)
{
	if (button.button == SDL_BUTTON_LEFT)
	{
		_left = dir;
	}
	if (button.button == SDL_BUTTON_RIGHT)
	{
#ifdef __EMSCRIPTEN__
		if (_controlPressed)
		{
			_left = dir;
		}
		else
		{
			_right = dir;
		}
#else
		_right = dir;
#endif
	}
}

void Mouse2D::updateSpinMatrix()
{
	double x = _lastX;
	double y = _lastY;
	
	convertCoords(&x, &y);
	glm::vec3 pos = glm::vec3(x, y, 0);
	
	glm::vec3 c = _centre;

	glm::vec4 tmp = _proj * glm::vec4(c, 1.);
	tmp /= tmp[3];

	glm::vec3 projAxis = glm::normalize(pos - glm::vec3(tmp));

	glm::vec3 rotAxis = glm::normalize(glm::cross(projAxis, glm::vec3(1, 0, 0)));
	_spin[0] = rotAxis;
	_spin[1] = glm::cross(rotAxis, projAxis);
	_spin[2] = projAxis;
	
//	_spin = glm::transpose(_spin);

}

void Mouse2D::mousePressEvent(double x, double y, SDL_MouseButtonEvent button)
{
	Scene::mousePressEvent(x, y, button);

	if (_modal != nullptr)
	{
		return;
	}

	interpretMouseButton(button, true);

	_lastX = x;
	_lastY = y;

	if ((_shiftPressed || _altPressed) && _makesSelections)
	{
		if (_shiftPressed)
		{
			_makingSelection = true;
			_reducingSelection = false;
		}
		else if (_altPressed)
		{
			_makingSelection = false;
			_reducingSelection = true;
		}
		_leftPos = x;
		_rightPos = x;
		_topPos = y;
		_bottomPos = y;
		updateSelectionBox();
	}

	updateSpinMatrix();
}

void Mouse2D::mouseMoveEvent(double x, double y)
{
	Scene::mouseMoveEvent(x, y);

	if (_modal != nullptr || _dragged != nullptr)
	{
		return;
	}

	if (_left && !_shiftPressed && !_controlPressed && !_altPressed && !_2D)
	{
		double dx = x - _lastX;
		double dy = y - _lastY;
		
		dx /= width();
		dy /= height();
		
		glm::vec4 start = _proj * glm::vec4(0, 0, -1, 1);
		glm::vec4 end = _proj * glm::vec4(dx, -dy, -1, 1);
		
		glm::vec3 move = start - end;
		move.z = 0;
		move = _spin * move;

		const float scale = 5;
		_camAlpha = move.x * scale;
		_camBeta =  move.y * scale;
		_camGamma = -move.z * scale;
		_camAlpha *= Window::aspect();
		
		updateCamera();
		updateSpinMatrix();
	}

	if (_makingSelection || _reducingSelection)
	{
		_rightPos = x;
		_bottomPos = y;
		updateSelectionBox();
		return;
	}

	if (_left && !_shiftPressed && _controlPressed)
	{
		double dx = x - _lastX;
		double dy = y - _lastY;

		_translation.x = dx / 50;
		_translation.y = -dy / 50;
		
		updateCamera();
	}

	if (_right && !_shiftPressed && !_controlPressed)
	{
		double dy = y - _lastY;

		_translation.z = dy / 2;
		
		updateCamera();
	}
	
	_lastX = x;
	_lastY = y;
	reslab();
}

void Mouse2D::regulariseBox()
{
	if (_leftPos > _rightPos)
	{
		float tmp = _leftPos;
		_leftPos = _rightPos;
		_rightPos = tmp;
	}

	if (_topPos > _bottomPos)
	{
		float tmp = _topPos;
		_topPos = _bottomPos;
		_bottomPos = tmp;
	}
}

void Mouse2D::mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button)
{
	Scene::mouseReleaseEvent(x, y, button);
	
	if ((_reducingSelection || _makingSelection) && _left)
	{
		_makingSelection = false;
		removeObject(_box);
		delete _box;
		_box = nullptr;
		
		regulariseBox();
		
		sendSelection(_topPos, _leftPos, _bottomPos, _rightPos, 
		              _reducingSelection);
		
		_reducingSelection = false;
	}

	if (_modal != nullptr)
	{
		return;
	}

	interpretMouseButton(button, false);
}

void Mouse2D::keyPressEvent(SDL_Keycode pressed)
{
	Scene::keyPressEvent(pressed);
	
	if (pressed == SDLK_d)
	{
		_manualFar -= 0.2;
	}
	else if (pressed == SDLK_f)
	{
		_manualFar += 0.2;
	}
	
	reslab();
	viewChanged();
}

void Mouse2D::updateSelectionBox()
{
	if (_box == nullptr)
	{
		_box = new SelectionBox();
		addObject(_box);
	}

	double x = _leftPos; double y = _topPos;
	convertToGLCoords(&x, &y);
	_box->setTopLeft(y, x);

	x = _rightPos; y = _bottomPos;
	convertToGLCoords(&x, &y);

	_box->setBottomRight(y, x);
}

void Mouse2D::sendSelection(float top, float left, float bottom, float right,
                            bool inverse)
{

}

void Mouse2D::reslab()
{
	if (!_slabbing)
	{
		return;
	}

	float dist = fabs(_centre.z);

	_nearSlab = 2 * dist / 3 - _manualFar;
	_farSlab = 3 * dist / 2 + _manualFar;
	if (_farSlab < _nearSlab + 1)
	{
		_farSlab = _nearSlab + 1;
	}
}
