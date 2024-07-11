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

#ifndef __vagabond__RouteExplorer__
#define __vagabond__RouteExplorer__

#include <thread>

#include "Display.h"
#include <vagabond/gui/elements/DragResponder.h>
#include <thread>

class PlausibleRoute;
class TextButton;
class Instance;
class GuiBalls;
class Slider;
class Route;
class Path;
class Atom;

class RouteExplorer : public Display, public DragResponder,
public Responder<Route>, public Responder<GuiBalls>
{
public:
	RouteExplorer(Scene *prev, PlausibleRoute *route);
	virtual ~RouteExplorer();
	
	void saveOver(Path *path)
	{
		_oldPath = path;
	}
	
	void setRestart(bool restart)
	{
		_restart = restart;
	}

	virtual void setup();
	virtual void doThings();
	void setupSlider();
	void clearLemons();
	
	virtual void sendObject(std::string tag, void *object);

	virtual void finishedDragging(std::string tag, double x, double y);
	virtual void buttonPressed(std::string tag, Button *button);

	virtual void interactedWithNothing(bool left, bool hover);
protected:
	void sendSelection(float t, float l, float b, float r, bool inverse);
private:
	void setupSave();
	void setupEditor();
	void setupFinish();
	void saveAndRestart();
	void setupClearLemons();

	void calculate();
	void startWithThreads(const int &thr);
	void handleDone();
	void pause();
	void demolishSlider();

	void prepareEmptySpaceMenu();
	void pickAtom(Atom *const &atom);

	void makeLemons();

	Route *_route = nullptr;
	PlausibleRoute *_plausibleRoute = nullptr;
	Path *_oldPath = nullptr;

	Instance *_instance = nullptr;

	Slider *_rangeSlider = nullptr;
	TextButton *_startPause = nullptr;
	TextButton *_saveAndExit = nullptr;
	std::thread *_worker = nullptr;

	std::atomic<bool> _watch{false};
	
	time_t _start = {};
	bool _first = true;
	static int _threads;
	bool _pausing = false;
	bool _restart = false;
	float _newScore = NAN;
	std::string _progressName;
	int _numberMade = 0;
	AtomGroup *_atoms = nullptr;
};

#endif
