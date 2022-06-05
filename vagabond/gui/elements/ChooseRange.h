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

#ifndef __vagabond__ChooseRange__
#define __vagabond__ChooseRange__

#include "Modal.h"
#include "Button.h"
#include "ButtonResponder.h"

class Slider;

class ChooseRange : virtual public Modal, public ButtonResponder, virtual public Button
{
public:
	ChooseRange(Scene *scene, std::string text, std::string tag,
	            ButtonResponder *sender);
	~ChooseRange();

	void setRange(float min, float max, float steps = 100);
	virtual void buttonPressed(std::string tag, Button *button);
	
	float min()
	{
		return _min;
	}
	
	float max()
	{
		return _max;
	}
private:
	Slider *_minSlider = nullptr;
	Slider *_maxSlider = nullptr;

	ButtonResponder *_sender = nullptr;
	float _min = 0;
	float _max = 0;
	float _step = 1;
};

#endif
