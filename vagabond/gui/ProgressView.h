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

#ifndef __vagabond__ProgressView__
#define __vagabond__ProgressView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Progressor.h>
#include <vagabond/core/Responder.h>
#include <map>
#include <mutex>
#include <vector>

class Text;

class ProgressViewResponder
{
public:
	virtual ~ProgressViewResponder() {};
	virtual void resume() = 0;
};

class ProgressView : public Scene,
public Responder<Progressor>
{
public:
	ProgressView(Scene *prev);
	virtual ~ProgressView();

	void attachToEnvironment();
	
	void setResponder(ProgressViewResponder *pvr)
	{
		_responder = pvr;
	}

	virtual void doThings();
	virtual void setup();
	virtual void finish();
	virtual void clickTicker(Progressor *progressor);
	virtual void sendObject(std::string tag, void *object);
private:
	Text *getText(std::string name, std::string str);

	std::map<std::string, Text *> _prog2Text;
	float _y = 0.3;

	ProgressViewResponder *_responder = nullptr;
	
	struct ProgressInfo
	{
		std::string name;
		int ticks;
	};

	std::mutex _mutex;
	std::vector<ProgressInfo> _queue;
	bool _finish = false;
};

#endif
