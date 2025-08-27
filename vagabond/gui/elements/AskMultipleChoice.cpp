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

#include "AskMultipleChoice.h"
#include "TextButton.h"

AskMultipleChoice::AskMultipleChoice(Scene *scene, std::string text, bool cancel)
: Modal(scene, 0.6, 0.4)
{
	Text *t = new Text(text);
	t->resize(0.8);
	t->setCentre(0.5, 0.35);
	addObject(t);

	setDismissible(false);
	
	if (cancel)
	{
		TextButton *tb = new TextButton("Cancel", this);
		tb->resize(0.8);
		auto hide_modal = [this]() { hide(); };

		tb->setReturnJob(hide_modal);
		tb->setLeft(0.35, 0.63);
		addObject(tb);
		_cancel = tb;
	}
}

void AskMultipleChoice::addChoice(const std::string &text, 
                                  const std::function<void()> &job)
{
	TextButton *tb = new TextButton(text, this);
	tb->resize(0.8);
	_jobs[text] = job;

	auto hide_and_job = [this, text]()
	{
		hide();
		_jobs[text]();
	};

	tb->setReturnJob(hide_and_job);
	tb->setLeft(0.35, _top);
	_boxes.push_back(tb);
	addObject(tb);
	
	_top += _inc;
}

void AskMultipleChoice::refresh()
{
	if (_boxes.size() > 3)
	{
		float height = 0.4 + _inc * (_boxes.size() - 3);
		makeFreshBox(0.6, height);
		_top = 0.43 - _inc / 2.;
		
		for (Renderable *r : _boxes)
		{
			r->setLeft(0.35, _top);
			_top += _inc;
		}
		_cancel->setLeft(0.35, 0.5 + height / 2 - _inc);
	}
}
