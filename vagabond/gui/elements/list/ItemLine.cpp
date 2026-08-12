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

#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/GLView.h>
#include "../ImageButton.h"
#include "../Window.h"
#include "ItemLine.h"
#include <vagabond/core/Item.h>
#include "LineGroup.h"

ItemLine::ItemLine(LineGroup *group, Item *item,
                   const std::function<void(Item *)> &job) 
: Box()
{
	if (item == nullptr)
	{
		throw std::runtime_error("Cannot make ItemLine from nullptr Item.");
	}

	if (group == nullptr)
	{
		throw std::runtime_error("Cannot make ItemLine from nullptr LineGroup.");
	}

	_item = item;
	_group = group;
	_job = job;

	setName(item->displayName() + " line");
	
	setup();
	
	_item->setResponder(this);
}

void ItemLine::addBranch()
{
	size_t depth = _item->depth();
	
	if (depth < 1)
	{
		return;
	}

	float branch_indent = (float)(depth) * _unitHeight;

	Image *image = new Image("assets/images/dot.png");
	image->resize(0.01);
	image->setLeft(branch_indent, 0.0);
	addObject(image);
	_dot = image;
}

void ItemLine::turnArrow()
{
	if (_item->collapsed() == _displayCollapse || _triangle == nullptr)
	{
		return;
	}
	
	_displayCollapse = _item->collapsed();

	float angle = (_displayCollapse ? -90 : +90);
	glm::mat3x3 mat = glm::mat3(1.);
	float &change = _displayCollapse ? mat[0][0] : mat[1][1];
	float &still = _displayCollapse ? mat[1][1] : mat[0][0];
	still = 1 / (double)Window::aspect();
	change = (double)Window::aspect();
	
	if (!_displayCollapse)
	{
		change = 1 / change;
		still = 1 / still;
	}

	glm::mat3x3 rot;
	rot = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(angle),
	                              glm::vec3(0., 0., -1.)));
	_triangle->rotateRoundCentre(rot);
	_triangle->rotateRoundCentre(mat);

	forceRender();
}

void ItemLine::update()
{
	if (_update)
	{
		// content first, not the arrow - addArrow() below positions the
		// arrow relative to _content's own current xy(), which is only
		// reliable once replaceContent() has actually run: an item that
		// transitions from no-arrow to arrow-bearing (e.g. gaining its
		// first child) triggers this same update() well after the
		// surrounding tree's own group-level offset (LineGroup::setLeft())
		// has already been applied once - addArrow(), if it ran first,
		// would build the arrow's own position from scratch using only
		// depth/_unitHeight, landing it in a stale reference frame that
		// hasn't picked up that offset, while _content (rebuilt fresh
		// every single update(), so always in sync) looks fine - exactly
		// the "arrow ends up far from the text" split this order avoids.
		replaceContent();

		if (shouldHaveArrow() && !_triangle)
		{
			addArrow();
		}

		turnArrow();
		_group->refreshGroups();
		_update = false;
	}
}

bool ItemLine::shouldHaveArrow()
{
	bool arrow = (_item->itemCount() > 0) && _item->canUnfold();
	return arrow;
}

void ItemLine::addArrow()
{
	bool arrow = shouldHaveArrow();
	_displayCollapse = _item->collapsed();

	if (arrow)
	{
		ImageButton *b = new ImageButton("assets/images/triangle.png", _group);
		b->rescale(0.025, 0.025);

		// positioned relative to _content's own current xy() - one
		// _unitHeight to its left, same y - rather than computing an
		// independent depth/_unitHeight-based position the way
		// replaceContent() does for _content: update() now always runs
		// replaceContent() before this, specifically so _content's xy()
		// is a reliable, already-correct reference to build from. An
		// independent computation looked fine for an item whose arrow
		// exists from construction (both would agree, since nothing has
		// repositioned the surrounding tree yet), but an item that only
		// gains its arrow later - e.g. on getting its first child, well
		// after LineGroup::setLeft() has already placed the surrounding
		// group somewhere - built the arrow's position from scratch in a
		// stale reference frame that never picked up that placement,
		// landing it far from (previously: on top of, before an earlier,
		// incomplete fix to this same issue) the text.
		glm::vec2 contentXY = _content ? _content->xy() : glm::vec2(0.f);
		b->setLeft(contentXY.x - _unitHeight, contentXY.y);

		auto toggle_button = [this]()
		{
			_item->toggleCollapsed();
			_group->topLevel()->reorganiseHeights();
		};

//		b->setReturnTag("toggle_" + _item->tag());
		b->setReturnJob(toggle_button);
		addObject(b);
		_displayCollapse = false;
		_triangle = b;
		turnArrow();
	}
	
	if (_dot)
	{
		removeObject(_dot);
		delete _dot;
		_dot = nullptr;
	}
}

void ItemLine::replaceContent()
{
	glm::vec2 xy{};
	bool reuse_old = false;

	if (_content)
	{
		xy = _content->xy();
		reuse_old = true;

		removeObject(_content);
		Window::setDelete(_content);
		_content = nullptr;
	}

	_content = displayRenderable(_group);
	_unitHeight = _content->maximalHeight() / 4;

	size_t depth = _item->depth();

	float start_indent = (float)(depth + 1) * _unitHeight;

	_content->setLeft(start_indent, 0.0);
	if (reuse_old)
	{
		_content->setArbitrary(xy.x, xy.y, Alignment::Left);
	}

	addObject(_content);
}

void ItemLine::setup()
{
	replaceContent();

	bool arrow = shouldHaveArrow();
	
	if (arrow)
	{
		addArrow();
	}
	else
	{
		addBranch();
	}
}

Renderable *ItemLine::displayRenderable(ButtonResponder *parent) const
{
	Box *text = nullptr;
	text = _item->customRenderable(parent, _job);

	auto wrap_job = [this]()
	{
		return [this]()
		{
			if (_item->selectJob())
			{
				_item->selectJob()(_gl->leftMouse());
			}
			else if (_job && _gl->leftMouse())
			{
				_job(_item);
			}
			else if (!_gl->leftMouse())
			{
				_group->makeMenu();
			}
		};
	};

	if (!text && _item->isEditable())
	{
		text = new TextEntry(_item->displayName(), parent);
	}
	else if (!text && _item->isSelectable())
	{
		TextButton *tb = new TextButton(_item->displayName(), parent);
		tb->setReturnJob(wrap_job());
		text = tb;
	}
	else if (!text && !_item->isSelectable())
	{
		// not delay=true (deferred texture creation, resolved later in
		// Text::render()) - ItemLine construction only ever happens
		// synchronously, as part of LineGroup's own construction, itself
		// always triggered by a UI action on the main thread, so there is
		// no thread-safety reason to defer here the way e.g. ProgressBar
		// (updated from a worker thread) genuinely needs to. The deferred
		// path was also landing this label at the same position as the
		// row's own arrow/dot icon instead of start_indent further right
		// (see replaceContent()) - not fully root-caused, but this sidesteps
		// it by building the texture immediately, the same way TextButton
		// (the selectable-item branch above, which never showed this
		// symptom) already does.
		text = new Text(_item->displayName());
	}

	text->resize(0.5);
	text->setName("content: " + _item->displayName());
	glm::vec3 c = _item->textColour();
	text->setColour(c.x, c.y, c.z);

	return text;
}


void ItemLine::respond()
{
	_update = true;
}

void ItemLine::doThings()
{
	update();

}

void ItemLine::setColour(float r, float g, float b)
{
	_content->setColour(r, g, b);
	_content->forceRender();
}
