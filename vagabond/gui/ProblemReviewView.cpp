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

#include "ProblemReviewView.h"
#include <vagabond/gui/elements/Text.h>

ProblemReviewView::ProblemReviewView(Scene *prev, 
                                     const std::vector<std::string> &messages)
: ListView(prev), _messages(messages)
{

}

void ProblemReviewView::setup()
{
	addTitle("Problems to resolve");

	ListView::setup();
}

size_t ProblemReviewView::lineCount()
{
	return _messages.size();
}

Renderable *ProblemReviewView::getLine(int i)
{
	Text *text = new Text(_messages[i]);
	text->setLeft(0, -0.1);
	text->resize(0.4);
	return text;
}
