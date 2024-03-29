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

#ifndef __vagabond__Toolkit__
#define __vagabond__Toolkit__

#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/ButtonResponder.h>

class Scene;

class Toolkit : public ImageButton, public ButtonResponder
{
public:
	Toolkit(Scene *scene);

	virtual void click(bool left = true);
	virtual void buttonPressed(std::string tag, Button *button);
private:
	void prepareProgress(int ticks, std::string text);
	Scene *_scene = nullptr;

};

#endif
