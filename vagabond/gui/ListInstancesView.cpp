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

#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Image.h>
#include "ChooseHeader.h"
#include "ListInstancesView.h"

ListInstancesView::ListInstancesView(Scene *prev) : ListView(prev)
{

}

void ListInstancesView::setup()
{
	addTitle("Include instances");
	ListView::setup();
}

size_t ListInstancesView::lineCount()
{
	return _fromList.size();
}

Renderable *ListInstancesView::getLine(int i)
{
	Box *b = new Box();
	{
		TickBoxes *tb = new TickBoxes(this, this);
		std::string text = _fromList[i];
		bool ticked = _fromTo[text].active;
		tb->addOption(text, text, ticked);
		tb->arrange(0, 0, 0.5, 0.1);
		if (_toList.size())
		{
			tb->setReturnJob
			([this, tb, text]()
			 {
				bool ticked = tb->isTicked(text);
				_fromTo[text].active = ticked;
				refresh();
			 });
		}

		b->addObject(tb);
		
		if (_fromTo[text].active)
		{
			Image *image = new Image("assets/images/axis.png");
			glm::mat3x3 rot = glm::mat3x3(glm::rotate(glm::mat4(1.), 
			                                          (float)deg2rad(-90),
			                                          glm::vec3(0., 0., -1.)));
			image->rotateRoundCentre(rot);
			image->rescale(0.012, 0.04);
			image->setCentre(0.35, 0);
			b->addObject(image);

			std::string inviteText = _fromTo[text].id;
			if (inviteText.length() == 0) { inviteText = "Choose..."; }
			TextButton *rhs = new TextButton(inviteText, this);
			rhs->setRight(0.6, 0);
			rhs->setReturnJob([this, text]()
			{
				_selected = text;

				ChooseHeader *ch = new ChooseHeader(this, true);
				ch->setResponder(this);
				ch->setHeaders(_toList);
				ch->setTitle("Set target instance from \"to\" model");
				ch->show();
			
			});
			b->addObject(rhs);
		}
	}

	return b;
}

void ListInstancesView::sendObject(std::string tag, void *object)
{
	_fromTo[_selected].id = tag;
	_fromTo[_selected].active = true;
	std::cout << _selected << " to " << tag << std::endl;
	refresh();
}

void ListInstancesView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "back")
	{
		sendResponse("instance_map", &_fromTo);
	}
	
	ListView::buttonPressed(tag, button);
}
