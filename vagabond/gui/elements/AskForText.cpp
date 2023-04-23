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

#include "AskForText.h"
#include "Scene.h"

AskForText::AskForText(Scene *scene, std::string text, std::string tag,
                       ButtonResponder *sender, TextEntry::Validation v)
: Modal(scene, 0.6, 0.4), Button(sender)
{
	setInert(true);

	Text *t = new Text(text);
	t->setCentre(0.5, 0.45);
	addObject(t);
	
	TextEntry *te = new TextEntry("enter", scene);
	te->setValidationType(v);
	te->setCentre(0.5, 0.5);
	te->setReturnTag("text");
	_text = te;
	addObject(te);

	addTwoButtons("Cancel", "cancel", "OK", "ok");
	setReturnTag(tag);
	
	setDismissible(true);
	te->click(true);
}

AskForText::~AskForText()
{
	_sender->unsetKeyResponder(_text);
	deleteObjects();
}

void AskForText::buttonPressed(std::string tag, Button *button)
{
	if (tag == "cancel")
	{
		hide();
	}

	if (tag == "ok")
	{
		hide();
		_sender->buttonPressed(AskForText::tag(), _text);
	}
}
