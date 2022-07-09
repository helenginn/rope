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

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Responder.h>
#include <mutex>

class GuiAtom;
class Display;

class SerialRefiner : public Scene, public Responder<Entity>, 
public Responder<Display>
{
public:
	SerialRefiner(Scene *prev, Entity *entity);
	
	void setExtra(const int extra)
	{
		_extra = extra;
	}
	
	void setRefineAll(bool all)
	{
		_all = all;
	}

	virtual ~SerialRefiner();

	virtual void setup();
	virtual void respond();
	virtual void doThings();
	virtual void entityDone();
	virtual void sendObject(std::string tag, void *object);
private:
	int _extra = 0;
	int _count = 0;
	bool _newModel = false;
	bool _all = false;
	Model *_model = nullptr;
	Display *_display = nullptr;
	Entity *_entity = nullptr;
	std::mutex _mutex;
};

#endif
