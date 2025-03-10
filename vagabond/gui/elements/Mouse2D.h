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

#ifndef __vagabond__Mouse2D__
#define __vagabond__Mouse2D__

#include "Scene.h"

class SelectionBox;

class Mouse2D : virtual public Scene
{
public:
	Mouse2D(Scene *prev = nullptr);
	virtual ~Mouse2D();

	virtual void mouseMoveEvent(double x, double y);
	virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void keyPressEvent(SDL_Keycode pressed);

	void setControls(const bool controls)
	{
		_controls = false;
	}
protected:
	void reslab();
	virtual void sendSelection(float t, float l, float b, float r,
	                           bool inverse);

	void setMakesSelections(bool makes = true)
	{
		_makesSelections = makes;
	}

	void interpretMouseButton(SDL_MouseButtonEvent button, bool dir);

	void updateSpinMatrix();
	void updateSelectionBox();
	
	void regulariseBox();
	
	glm::mat3x3 _spin;

	bool _controls = true;
	bool _makesSelections = false;
	
	bool _2D = false;
	bool _makingSelection = false;
	bool _reducingSelection = false;
	float _topPos = 0;
	float _leftPos = 0;
	float _rightPos = 0;
	float _bottomPos = 0;
	
	float _manualFar = 0;
	
	SelectionBox *_box = nullptr;
	bool _slabbing = false;

};

#endif
