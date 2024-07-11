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

#ifndef __vagabond__SerialRefiner__
#define __vagabond__SerialRefiner__

#include <vagabond/core/SerialJob.h>
#include <vagabond/core/RopeJob.h>
#include <time.h>
#include <mutex>
#include <set>

#include "Display.h"

class SerialRefineJob;

class Model;
class Entity;

class SerialRefiner : public Display, public SerialJobResponder<Model *>
{
public:
	SerialRefiner(Scene *prev, Entity *entity);

	virtual ~SerialRefiner();
	
	void setJobType(rope::RopeJob job);
	
	void setRefineList(std::set<Model *> models);

	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button);

	virtual void attachObject(Model *model);
	virtual void detachObject(Model *model);
	void updateObject(Model *model, int idx);

	virtual void finishedObjects();

	virtual void doThings();
	virtual void entityDone();
private:
	void loadModelIntoDisplay(Model *model);
	void showThreads();
	void start();
	void showSummary();
	void updateText(Model *model, int idx);
	
	enum Task
	{
		Attach,
		Detach,
		Finish,
		UpdateText,
	};
	
	struct Update
	{
		Task task;
		Model *model;
		int idx;
	};
	
	std::vector<Update> _updates;
	void addUpdate(Update up);
	void processUpdate(Update &update);

	SerialRefineJob *_handler = nullptr;
	std::vector<Text *> _threadReporters;

	std::mutex _mutex;
	std::condition_variable _cv;
	time_t _start{};
	AtomGroup *_displayAtoms = nullptr;
};

#endif
