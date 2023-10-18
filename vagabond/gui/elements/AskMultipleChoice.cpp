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

#include "AskMultipleChoice.h"
#include "TextButton.h"

AskMultipleChoice::AskMultipleChoice(Scene *scene, std::string text,
                                     std::string tag, ButtonResponder *sender) :
Modal(scene, 0.6, 0.4)
{
	Text *t = new Text(text);
	t->resize(0.8);
	t->setCentre(0.5, 0.35);
	addObject(t);

	_sender = sender;
	_tag = tag;

	setDismissible(false);
}

void AskMultipleChoice::addChoice(std::string text, std::string tag)
{
	TextButton *tb = new TextButton(text, this);
	tb->resize(0.8);
	tb->setReturnTag(tag);
	tb->setLeft(0.35, _top);
	addObject(tb);
	
	_top += 0.07;
}

void AskMultipleChoice::buttonPressed(std::string tag, Button *button)
{
	hide();
	_sender->buttonPressed(_tag + "_" + tag);
}
