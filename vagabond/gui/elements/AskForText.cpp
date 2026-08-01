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
	_preamble = t;
	
	_scene = scene;
	TextEntry *te = new TextEntry("enter", scene, scene);
	te->setValidationType(v);
	te->setCentre(0.5, 0.5);
	te->setReturnTag("text");
	_text = te;
	_text->HasResponder<Responder<TextEntry>>::setResponder(this);
	addObject(te);
	float height = te->maximalHeight();
	stretchToFit(te);

	addTwoButtons("Cancel", "cancel", "OK", "ok");
	setReturnTag(tag);
	
	setDismissible(true);
	te->click(true);
}

void AskForText::setDefaultText(const std::string &text)
{
	_text->setScratch(text);

	// showInsert() (not a plain setText() call - see there) both
	// rebuilds the correctly-sized quad AND re-snapshots
	// _unselectedVertices to match it. Skipping the second part (as a
	// bare setText() call would) leaves _unselectedVertices holding
	// whatever the entry looked like when click(true) highlighted it in
	// the constructor above - back when _scratch was still empty, i.e.
	// a single "_" cursor's width. That stale snapshot is invisible
	// until the next unMouseOver() (Renderable::setHighlighted(false))
	// restores _vertices from it, which is why this bug only shows up
	// after the mouse moves off the entry, not immediately.
	_text->showInsert();

	// showInsert() rebuilds the geometry correctly, but this typically
	// runs before the scene has ever rendered this brand new modal even
	// once, so there is no dirty/repaint flag telling it to actually
	// redraw yet - without this, the correct geometry sits unpainted
	// until some unrelated event forces a repaint.
	_scene->viewChanged();
}

void AskForText::allowCapitals(bool capitals)
{
	_text->allowCapitals(capitals);
}

void AskForText::allowMultiLine(bool allow)
{
	_text->allowMultiLine(allow);
	if (allow)
	{
		_text->setStretchFunction([this](TextEntry *te)
		                          {
			                         stretchToFit(te);
			                      });
	}
	else
	{
		_text->setStretchFunction({});
	}
}

void AskForText::stretchToFit(TextEntry *te)
{
	float delta_height = (te->maximalHeight()) / 2;
	float adjusted = 0.4 + delta_height / 2;
	float preamble = 0.45 - delta_height / 2;
	makeFreshBox(0.6, adjusted);
	_preamble->setCentre(0.5, preamble);
}

AskForText::~AskForText()
{

}

void AskForText::respond()
{
	buttonPressed("ok", nullptr);
}

void AskForText::buttonPressed(std::string tag, Button *button)
{
	if (tag == "cancel")
	{
		if (_cancel)
		{
			_cancel();
		}
		hide();
	}

	if (tag == "ok")
	{
		hide();
		
		if (_job)
		{
			_job(_text->scratch());
		}
		else
		{
			_sender->buttonPressed(AskForText::tag(), _text);
		}
	}
}

void AskForText::keyPressed(SDL_Keycode other)
{
	if (other == SDLK_RETURN)
	{
		buttonPressed("ok", nullptr);
	}
}

void AskForText::keyPressed(char key)
{

}
