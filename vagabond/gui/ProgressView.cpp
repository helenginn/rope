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

}

void ProgressView::attachToEnvironment()
{
	Environment::env().setProgressResponder(this);
}

Text *ProgressView::getText(std::string name, std::string str)
{
	if (_prog2Text.count(name))
	{
		_prog2Text[name]->setText(str);
		return _prog2Text[name];
	}

	Text *t = new Text(str, Font::Thin);
	t->setCentre(0.5, _y);
	_y += 0.08;
	_prog2Text[name] = t;
	addObject(t);

	return t;
}

void ProgressView::doThings()
{
	_mutex.lock();
	
	if (_finish)
	{
		_mutex.unlock();
		return;
	}
	
	for (const ProgressInfo &info : _queue)
	{
		int ticks = info.ticks;
		std::string str = "Loaded " + i_to_str(ticks) + " ";
		str += info.name;
		getText(info.name, str);
		viewChanged();
	}

	_queue.clear();

	_mutex.unlock();
}

void ProgressView::clickTicker(Progressor *progressor)
{
	_mutex.lock();
	ProgressInfo info{progressor->progressName(), progressor->ticks()};
	_queue.push_back(info);
	_mutex.unlock();
}

void ProgressView::sendObject(std::string tag, void *object)
{
	if (tag == "tick")
	{
		Progressor *view = static_cast<Progressor *>(object);
		clickTicker(view);
	}
	else if (tag == "finish")
	{
		finish();
	}
}

void ProgressView::finish()
{
	std::lock_guard<std::mutex> lock(_mutex);

	_finish = true;

	if (_responder)
	{
		_responder->resume();
	}
	else
	{
		back();
	}
}
