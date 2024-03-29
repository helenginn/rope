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

#ifndef __vagabond__AskForText__
#define __vagabond__AskForText__

#include "Modal.h"
#include "Button.h"
#include "TextEntry.h"
#include "KeyResponder.h"

class TextEntry;

/** will call buttonPressed(tag, _text) - text is available through
 * casting to TextEntry and calling the scratch() geter */

class AskForText : public Modal, public Button, public KeyResponder,
public Responder<TextEntry>
{
public:
	AskForText(Scene *scene, std::string text, 
	           std::string tag, ButtonResponder *sender,
	           TextEntry::Validation v = TextEntry::None);
	
	void allowCapitals(bool capitals);
	virtual void respond();

	virtual ~AskForText();
	virtual void buttonPressed(std::string tag, Button *button);
	virtual void keyPressed(SDL_Keycode other);
	virtual void keyPressed(char key);
private:
	TextEntry *_text;

};

#endif
