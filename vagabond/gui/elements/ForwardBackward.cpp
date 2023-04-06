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

#include "ForwardBackward.h"
#include "ImageButton.h"

ForwardBackward::ForwardBackward(Scene *prev) : Scene(prev)
{

}

ForwardBackward::~ForwardBackward()
{
	deleteObjects(true);
}

void ForwardBackward::buttonPressed(std::string tag, Button *button)
{	
	if (tag == "scroll_forward")
	{
		_start += unitsPerPage();
		refresh();
	}
	if (tag == "scroll_back")
	{
		_start -= unitsPerPage();
		if (_start < 0)
		{
			_start = 0;
		}
		refresh();
	}
	
	Scene::buttonPressed(tag, button);
}

void ForwardBackward::scrollBackButton()
{
	ImageButton *bb = ImageButton::arrow(+90., this);
	bb->setCentre(0.1, 0.8);
	bb->setReturnTag("scroll_back");
	addTempObject(bb);
}

void ForwardBackward::scrollForwardButton()
{
	ImageButton *fb = ImageButton::arrow(-90., this);
	fb->setCentre(0.9, 0.8);
	fb->setReturnTag("scroll_forward");
	addTempObject(fb);
}

