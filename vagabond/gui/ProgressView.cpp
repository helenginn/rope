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

#include "ProgressView.h"
#include <vagabond/core/Environment.h>
#include <vagabond/core/Progressor.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/Text.h>

ProgressView::ProgressView(Scene *prev) : Scene(prev)
{

}

void ProgressView::setup()
{
	addTitle("Loading...");
	hideBackButton();

}

ProgressView::~ProgressView()
{
	Environment::env().setProgressResponder(nullptr);
}

void ProgressView::attachToEnvironment()
{
	Environment::env().setProgressResponder(this);
}

Text *ProgressView::getText(Progressor *p, std::string str)
{
	if (_prog2Text.count(p))
	{
		_prog2Text[p]->setText(str);
		return _prog2Text[p];
	}

	Text *t = new Text(str, true);
	t->setCentre(0.5, _y);
	_y += 0.08;
	_prog2Text[p] = t;
	addObject(t);

	return t;
}

void ProgressView::clickTicker(Progressor *progressor)
{
	int ticks = progressor->ticks();
	std::string str = "Loaded " + i_to_str(ticks) + " ";
	str += progressor->progressName();
	Text *text = getText(progressor, str);
}

void ProgressView::finish()
{
	if (_responder)
	{
		_responder->resume();
	}
	else
	{
		back();
	}
}
