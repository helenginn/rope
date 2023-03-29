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

#ifndef __vagabond__ItemLine__
#define __vagabond__ItemLine__

#include "../Box.h"

class LineGroup;
class ImageButton;
class Item;

class ItemLine : public Box
{
public:
	ItemLine(LineGroup *group, Item *item);

	void update();
private:
	void addBranch();
	void addArrow();
	void turnArrow();
	/** prepare content of box */
	void setup();
	
	float _unitHeight = 0.;
	
	bool _displayCollapse = false;

	Item *_item = nullptr;
	LineGroup *_group = nullptr;
	ImageButton *_triangle = nullptr;
};

#endif
