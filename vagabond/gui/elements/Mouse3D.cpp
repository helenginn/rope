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
#include "Mouse3D.h"

Mouse3D::Mouse3D(Scene *prev) : Scene(prev)
{

}

Mouse3D::~Mouse3D()
{

}

void Mouse3D::interpretMouseButton(SDL_MouseButtonEvent button, bool dir)
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

void Mouse3D::updateSpinMatrix()
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

void Mouse3D::mousePressEvent(double x, double y, SDL_MouseButtonEvent button)
{
	Scene::mousePressEvent(x, y, button);

	if (_modal != nullptr)
	{
		return;
	}

	interpretMouseButton(button, true);

	_lastX = x;
	_lastY = y;

	updateSpinMatrix();
}

void Mouse3D::mouseMoveEvent(double x, double y)
{
	Scene::mouseMoveEvent(x, y);

	if (_modal != nullptr)
	{
		return;
	}


	if (_left && !_shiftPressed && !_controlPressed)
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
}

void Mouse3D::mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button)
{
	Scene::mouseReleaseEvent(x, y, button);

	if (_modal != nullptr)
	{
		return;
	}

	interpretMouseButton(button, false);
}

void Mouse3D::interpretControlKey(SDL_Keycode pressed, bool dir)
{
	if (pressed == SDLK_LCTRL)
	{
		_controlPressed = dir;
	}

	if (pressed == SDLK_LSHIFT)
	{
		_shiftPressed = dir;
	}
}

void Mouse3D::keyPressEvent(SDL_Keycode pressed)
{
	interpretControlKey(pressed, true);
}

void Mouse3D::keyReleaseEvent(SDL_Keycode pressed)
{
	interpretControlKey(pressed, false);
}
