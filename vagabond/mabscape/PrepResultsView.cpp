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

#include "PrepResultsView.h"
#include <vagabond/gui/elements/Box.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TextButton.h>

PrepResultsView::PrepResultsView(Scene *prev, 
                                 const std::vector<PrepMessage> &messages,
                                 const std::function<void(Scene *)> &next)
: ListView(prev), _messages(messages), _next(next)
{

}

void PrepResultsView::setup()
{
	addTitle("Completed " + std::to_string(_messages.size()) + " jobs");

	TextButton *tb = new TextButton("Next", this);
	tb->setRight(0.9, 0.9);
	tb->setReturnJob([this]()
	{
		if (_next)
		{
			_next(this);
		}
	});
	addObject(tb);

	ListView::setup();
}

size_t PrepResultsView::lineCount()
{
	return _messages.size();
}

Renderable *PrepResultsView::getLine(int i)
{
	const PrepMessage &message = _messages[i];

	Box *b = new Box();

	if (message.warning)
	{
		ImageButton *warning = new ImageButton("assets/images/warning.png", 
		                                       nullptr);
		warning->resize(0.04);
		warning->setLeft(0.0, 0.0);

		if (message.show)
		{
			auto show = message.show;
			warning->setReturnJob([this, show]() { show(this); });
		}

		b->addObject(warning);
	}

	Text *text = new Text(message.text);
	text->resize(0.6);
	text->setLeft(0.05, 0.0);
	b->addObject(text);

	return b;
}
