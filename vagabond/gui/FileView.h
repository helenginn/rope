// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__FileView__
#define __vagabond__FileView__

#include "Scene.h"
#include "FileManager.h"

class ImageButton;

class FileView : public Scene
{
public:
	FileView(Scene *prev, int start = 0);
	
	virtual ~FileView();
	virtual void setup();
	void refreshFiles();
	virtual void buttonPressed(std::string tag, Button *button = NULL);
private:
	void scrollBackButton();
	void scrollForwardButton();
	void loadFilesFrom(int start, int num);
	FileManager *_manager;

	std::vector<Renderable *> _temps;
	int _start;
};

#endif
