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

#ifndef __vagabond__LineGroup__
#define __vagabond__LineGroup__

#include "../Box.h"
#include "../Button.h"
#include "../ButtonResponder.h"

class Item;
class Menu;
class Scene;
class ItemLine;

class LineGroup : public ButtonResponder, public Button
{
public:
	LineGroup(Item *item, Scene *sender = nullptr);

	void setup();
	void refreshGroups();

	virtual void buttonPressed(std::string tag, Button *button);
private:
	void fixUpperCorner(Renderable *r, float x = 0, float y = 0);
	LineGroup(Item *item, LineGroup *topLevel);
	void initialise(Item *item, LineGroup *top);
	Menu *prepareMenu();
	void setupGroups();
	void reorganiseHeights();
	void reorganiseGroups();
	void resetGroups();

	Item *_item = nullptr;
	ItemLine *_line = nullptr;
	
	LineGroup *_topLevel = nullptr;
	LineGroup *_parent = nullptr;
	Scene *_scene = nullptr;

	std::vector<LineGroup *> _groups;
};

#endif
