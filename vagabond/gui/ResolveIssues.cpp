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

#include "ResolveIssues.h"
#include <vagabond/core/FixIssues.h>

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/BadChoice.h>

ResolveIssues::ResolveIssues(Scene *prev, FixIssues *fixer) : ListView(prev)
{
	_fixer = fixer;
}

void ResolveIssues::setup()
{
	addTitle("Resolve issues");
	
	{
		TextButton *tb = new TextButton("Resolve", this);
		tb->setReturnTag("resolve");
		tb->setRight(0.9, 0.1);
		_resolve = tb;

		if (!_fixer->done())
		{
			tb->setAlpha(-.5f);
		}
		
		addObject(tb);
	}

	ListView::setup();
}

size_t ResolveIssues::lineCount()
{
	return _fixer->issueCount();
}

Renderable *ResolveIssues::getLine(int i)
{
	if (_fix.count(i) == 0)
	{
		std::cout << "setting " << i << " to true" << std::endl;
		_fix[i] = true;
	}

	Box *b = new Box();
	{
		TickBoxes *tb = new TickBoxes(this, this);
		std::string text = _fixer->issueMessage(i);
		std::string tag = "issue_" + std::to_string(i);
		tb->addOption(text, tag, _fix.at(i));
		tb->arrange(0, 0, 1.0, 0.1);
		b->addObject(tb);
	}

	return b;
}

void ResolveIssues::refresh()
{
	if (_resolve && _fixer->done())
	{
		_resolve->setAlpha(.0f);
		_resolve->forceRender(true, false);
	}

	ListView::refresh();
}

void ResolveIssues::respond()
{
	for (int i = _fix.size(); i < _fixer->issueCount(); i++)
	{
		_fix[i] = true;
	}


	_mustRefresh = true;
}

void ResolveIssues::resolveIssues()
{
	int count = 0;
	for (size_t i = 0; i < _fixer->issueCount(); i++)
	{
		if (_fix[i])
		{
			_fixer->fixIssue(i);
			count++;
		}
	}

	back();
	BadChoice *bc = new BadChoice(_previous, "Resolved " + std::to_string(count)
	                              + " selected issues.");
	_previous->setModal(bc);
}

void ResolveIssues::buttonPressed(std::string tag, Button *button)
{
	std::string end = Button::tagEnd(tag, "issue_");

	if (end.length())
	{
		int idx = atoi(end.c_str());
		TickBoxes *ch = static_cast<TickBoxes *>(button);
		_fix[idx] = !_fix[idx];
		ch->tick(tag, _fix[idx]);
	}
	else if (tag == "resolve" && _fixer->done())
	{
		resolveIssues();
	}
	else
	{
		ListView::buttonPressed(tag, button);
	}
}
