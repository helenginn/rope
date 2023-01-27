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

#include "SerialRefiner.h"
#include "Display.h"
#include "GuiAtom.h"
#include <vagabond/core/Model.h>
#include <vagabond/core/Environment.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <sstream>

SerialRefiner::SerialRefiner(Scene *prev, Entity *entity) : Scene(prev),
Display(prev)
{
	_handler = new SerialJob(entity, this);
	_handler->setThreads(1);
	setOwnsAtoms(false);
	setControls(false);
}

SerialRefiner::~SerialRefiner()
{

}

void SerialRefiner::start()
{
	_handler->setup();
	size_t count = _handler->modelCount();
	addTitle("Refining " + i_to_str(count) + " models...");
	showThreads();
	
	_start = ::time(nullptr);
	_handler->start();

	hideBackButton();

}

void SerialRefiner::setup()
{
#ifndef __EMSCRIPTEN__
	std::string str = "Choose number of threads";
	ChooseRange *cr = new ChooseRange(this, str, "choose_threads", this);
	cr->setDefault(4);
	cr->setRange(0, 32, 32);
	setModal(cr);
#else
	start();
#endif
}

void SerialRefiner::buttonPressed(std::string tag, Button *button)
{
	if (tag == "choose_threads")
	{
		ChooseRange *cr = static_cast<ChooseRange *>(button->returnObject());
		float num = cr->max();
		int threads = lrint(num);
		_handler->setThreads(threads);
		start();
	}
	else if (tag == "save_back")
	{
		Environment::env().save();
		back();
	}
	
	Display::buttonPressed(tag, button);
}

void SerialRefiner::setRefineList(std::set<Model *> models)
{
	std::vector<Model *> vec;
	
	for (Model *m : models)
	{
		vec.push_back(m);
	}

	_handler->setModelList(vec);
}

void SerialRefiner::setJobType(rope::RopeJob job)
{
	_handler->setRopeJob(job);
}

void SerialRefiner::entityDone()
{
	time_t end = ::time(nullptr);
	double diff = ::difftime(end, _start);
	std::cout << "diff = " << diff << std::endl;
	int mins = (int)(diff / 60);
	int seconds = diff - (mins * 60);

	std::ostringstream ss;
	ss << mins << "m " << seconds << "s";

	{
		Text *text = new Text("Done!");
		text->setLeft(0.3, 0.4);
		addObject(text);
	}
	{
		Text *text = new Text("Total time: " + ss.str());
		text->setLeft(0.3, 0.5);
		addObject(text);
	}
	
	{
		TextButton *text = new TextButton("Save and return", this);
		text->setReturnTag("save_back");
		text->setCentre(0.5, 0.6);
		addObject(text);
	}

}

void SerialRefiner::addUpdate(Update up)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_updates.push_back(up);
	/* will be released on main thread by doThings() */
	_cv.wait(lock);
}

void SerialRefiner::loadModelIntoDisplay(Model *model)
{
	AtomGroup *atoms = model->currentAtoms();

	if (atoms)
	{
		int count = _handler->finishedCount() + 1;
		int total = _handler->modelCount();

		std::ostringstream ss;
		ss << "Refining " << model->name() << " (" << count << "/" <<
		total << ")" << std::endl;

		addTitle(ss.str());
		loadAtoms(atoms);
		guiAtoms()->setDisableRibbon(false);
		guiAtoms()->setDisableBalls(true);
	}
}

void SerialRefiner::dismantleDisplay()
{
	stop();
	_atoms = nullptr;
}

void SerialRefiner::showSummary()
{
	int total = _handler->modelCount();
	addTitle("Refined " + std::to_string(total) + " models.");
	entityDone();
	showBackButton();
}

void SerialRefiner::updateText(Model *model, int idx)
{
	std::string text = "T#" + std::to_string(idx);
	
	if (model)
	{
		text += ": " + model->name();
	}
	else
	{
		text += ": idle";
	}

	_threadReporters[idx]->setText(text);
}

void SerialRefiner::processUpdate(Update &update)
{
	if (update.task == Attach)
	{
		loadModelIntoDisplay(update.model);
	}
	else if (update.task == Detach)
	{
		dismantleDisplay();
	}
	else if (update.task == Finish)
	{
		showSummary();
	}
	else if (update.task == UpdateText)
	{
		updateText(update.model, update.idx);
	}
}

void SerialRefiner::doThings()
{
	std::lock_guard<std::mutex> lock(_mutex);
	
	for (Update &update : _updates)
	{
		processUpdate(update);
	}

	_updates.clear();

	_cv.notify_all(); /* releases worker threads to act on models */
}

void SerialRefiner::attachModel(Model *model)
{
	addUpdate(Update{Attach, model, 0});
}

void SerialRefiner::updateModel(Model *model, int idx)
{
	addUpdate(Update{UpdateText, model, idx});
}

void SerialRefiner::detachModel(Model *model)
{
	addUpdate(Update{Detach, model, 0});
}

void SerialRefiner::finishedModels()
{
	addUpdate(Update{Finish, nullptr, 0});
}

void SerialRefiner::showThreads()
{
	float top = 0.1;
	float inc = 0.06;
	for (size_t i = 0; i < _handler->threadCount(); i++)
	{
		Text *t = new Text("T#" + std::to_string(i));
		t->setRight(0.9, top);
		addObject(t);
		top += inc;
		_threadReporters.push_back(t);
	}
}
