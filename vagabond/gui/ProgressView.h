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
#include <map>

class Text;

class ProgressViewResponder
{
public:
	virtual ~ProgressViewResponder() {};
	virtual void resume() = 0;
};

class ProgressView : public Scene, public ProgressResponder
{
public:
	ProgressView(Scene *prev);
	~ProgressView();

	void attachToEnvironment();
	
	void setResponder(ProgressViewResponder *pvr)
	{
		_responder = pvr;
	}

	virtual void setup();
	virtual void finish();
	virtual void clickTicker(Progressor *progressor);
private:
	Text *getText(Progressor *p, std::string str);

	std::map<Progressor *, Text *> _prog2Text;
	float _y = 0.3;

	ProgressViewResponder *_responder = nullptr;

};

#endif
