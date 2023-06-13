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

#ifndef __vagabond__AskMultipleChoice__
#define __vagabond__AskMultipleChoice__

#include "Modal.h"
#include "ButtonResponder.h"

class AskMultipleChoice : public Modal
{
public:
	AskMultipleChoice(Scene *scene, std::string text, std::string tag, 
	                  ButtonResponder *sender);
	
	void addChoice(std::string text, std::string tag);
	
	virtual ~AskMultipleChoice() {};

	virtual void buttonPressed(std::string tag, Button *button);

private:
	ButtonResponder *_sender;
	std::string _tag;

	float _top = 0.43;

};

#endif
