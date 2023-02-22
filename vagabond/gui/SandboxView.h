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

#ifndef __vagabond__SandboxView__
#define __vagabond__SandboxView__

#include <vagabond/gui/Display.h>
#include <vagabond/core/Responder.h>
#include <thread>

class Text;
class CyclicView;

class SandboxView : public Display, public Responder<CyclicView>
{
public:
	SandboxView(Scene *prev);

	void stop();
	virtual ~SandboxView();
	virtual void setup();
	virtual void doThings();
	virtual void respond();
	virtual void updateInfo();

	virtual void buttonPressed(std::string tag, Button *button = nullptr);

	virtual void mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button);
private:
	CyclicView *_cyclic = nullptr;
	Text *_angles = nullptr;
	Text *_lengths = nullptr;
	int _cycle = 0;

	std::thread *_worker = nullptr;
};

#endif
