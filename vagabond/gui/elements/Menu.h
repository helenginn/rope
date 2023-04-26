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

#ifndef __vagabond__Menu__
#define __vagabond__Menu__

#include "Modal.h"
#include "Button.h"
#include "ButtonResponder.h"

class TextButton;
class Renderable;

class Menu : public Modal, public Button
{
public:
	Menu(Scene *scene, ButtonResponder *br = nullptr, std::string prefix = "");
	~Menu();

	TextButton *addOption(std::string text, std::string tag = "");
	void setup(double x, double y, double resize = 1);
	void setup(Renderable *r, double resize = 1);
	void buttonPressed(std::string tag, Button *button = nullptr);
private:
	void optionLimits(double &width, double &height);
	std::vector<TextButton *> _options;
	std::string _prefix;
};

#endif
