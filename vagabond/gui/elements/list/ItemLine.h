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
#include <vagabond/core/Responder.h>

class LineGroup;
class ImageButton;
class Image;
class Menu;
class Item;

class ItemLine : public Box, public Responder<Item>
{
public:
	ItemLine(LineGroup *group, Item *item,
	         const std::function<void(Item *)> &job);

	void update();
	
	void setColour(float r, float g, float b);
	
	/** return renderable for Item. Default is display name.
	 * Renderable should be top-left justified i.e. setLeft(0.0, 0.0); */
	virtual Renderable *displayRenderable(ButtonResponder *parent) const;
	
	virtual void doThings();

	LineGroup *group()
	{
		return _group;
	}
protected:
	virtual void respond();
private:
	void addBranch();
	void addArrow();
	void turnArrow();
	
	bool shouldHaveArrow();
	/** number of lines the display name is broken into */
	size_t nameLines() const;
	/** Each line of the list is placed by the centre of its text, on the top
	 * of its slot, so text of several lines would stick out above the slot and
	 * leave a gap below it. This is how far (down, in screen fractions) the
	 * text of a name over several lines is put, for its top to be where a
	 * single line would have its top: zero for a single line. */
	float contentDrop();
	/** how far up from the centre of the content (negative y) the arrow and
	 * dot are put to be level with its first line: zero for a single line */
	float firstLineShift();
	void replaceContent();
	/** prepare content of box */
	void setup();
	
	float _unitHeight = 0.;
	
	bool _displayCollapse = false;
	std::atomic<bool> _update{false};

	Item *_item = nullptr;
	LineGroup *_group = nullptr;
	ImageButton *_triangle = nullptr;
	Image *_dot = nullptr;
	Renderable *_content = nullptr;
    std::function<void(Item *)> _job{};
};

#endif
