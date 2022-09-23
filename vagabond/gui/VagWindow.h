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

#ifndef __vagabond__VagWindow__
#define __vagabond__VagWindow__

#include <vagabond/gui/elements/Window.h>
#include "MainMenu.h"

class Dictator;

class VagWindow : public Window//, public ProgressViewResponder
{
public:
	VagWindow();
	virtual void setup(int argc, char **argv);
	void setup_special();
	virtual void resume();
	virtual void mainThreadActivities();
	
	static VagWindow *window()
	{
		return static_cast<VagWindow *>(_myWindow);
	}
	
	static Dictator *dictator()
	{
		return _dictator;
	}
	
	static void addJob(std::string str);

	void prepareProgressView();
	
	void prepareProgressBar(int ticks, std::string text = "");
private:
	static Dictator *_dictator;
	MainMenu *_menu = nullptr;
	bool _resume = false;

};

#endif
