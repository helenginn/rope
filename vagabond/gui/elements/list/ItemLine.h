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
	/** how far up from the middle of the content the centre of its first
	 * line is (negative y): zero for a single line, so the arrow and dot
	 * stay at the middle of the text, otherwise they go with the first line */
	float firstLineShift();
	/** An ItemLine is aligned by the mean of the centres of what it holds, so
	 * a marker moved off the middle of the text drags the whole line with it.
	 * This adds a hidden counterweight (disabled, so it adds nothing to the
	 * size) on the other side of the text's middle, to put that mean back. */
	void balanceMarker(float x, float y);
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
	Image *_balance = nullptr;
	Renderable *_content = nullptr;
    std::function<void(Item *)> _job{};
};

#endif
