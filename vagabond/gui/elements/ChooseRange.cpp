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

#include "ChooseRange.h"
#include "Scene.h"
#include "Slider.h"
#include "TextButton.h"

ChooseRange::ChooseRange(Scene *scene, std::string text, std::string tag,
                         ButtonResponder *sender, bool both)
: Modal(scene, 0.6, 0.4), Button(scene)
{
	setInert(true);
	Text *t = new Text(text);
	t->setCentre(0.5, 0.45);
	addObject(t);
	
	addTwoButtons("Cancel", "cancel", "OK", "ok");
	
	_both = both;
	_sender = sender;
	setReturnTag(tag);

	setDismissible(true);
}

void ChooseRange::setRange(float min, float max, float steps)
{
	_min = min;
	_max = max;
	_step = (max - min) / steps;

	{
		Slider *s = new Slider();
		s->resize(0.5);
		s->setup("Range", min, max, _step);
		s->setStart(0., 0.);
		s->setCentre(0.5, 0.5);
		_minSlider = s;
		addObject(s);
	}

	if (_both)
	{
		Slider *s = new Slider();
		s->resize(0.5);
		s->setup("Range", min, max, _step);
		s->setStart(1., 0.);
		s->setCentre(0.5, 0.5);
		_maxSlider = s;
		addObject(s);
	}
}

void ChooseRange::buttonPressed(std::string tag, Button *button)
{
	if (tag == "cancel")
	{
		hide();
		return;
	}
	_min = _minSlider->value();
	
	if (_both)
	{
		_max = _maxSlider->value();
	}
	else
	{
		_max = _min;
	}
	
	if (_min > _max)
	{
		float tmp = _min;
		_min = _max;
		_max = tmp;
	}

	setReturnObject(this);
	_sender->buttonPressed(Button::tag(), this);

	hide();
}

ChooseRange::~ChooseRange()
{
	deleteObjects();
}
