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

#include "Display.h"
#include <vagabond/gui/elements/DragResponder.h>

class PlausibleRoute;
class TextButton;
class Instance;
class Slider;
class Route;

class RouteExplorer : public Display, public DragResponder,
public Responder<Route>
{
public:
	RouteExplorer(Scene *prev, PlausibleRoute *route);
	virtual ~RouteExplorer();

	virtual void setup();
	virtual void doThings();
	void setupSlider();
	
	virtual void sendObject(std::string tag, void *object);

	virtual void finishedDragging(std::string tag, double x, double y);
	virtual void buttonPressed(std::string tag, Button *button);
private:
	void setupSave();
	void setupFinish();
	void setupSettings();
	Route *_route = nullptr;
	PlausibleRoute *_plausibleRoute = nullptr;
	Instance *_instance = nullptr;

	Slider *_rangeSlider = nullptr;
	TextButton *_startPause = nullptr;
	std::thread *_worker = nullptr;

	std::atomic<bool> _watch{false};
	
	int _numTicks = -1;
	float _newScore = NAN;
	std::string _progressName;
};

#endif
