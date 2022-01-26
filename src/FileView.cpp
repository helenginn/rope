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

#include "Display.h"
#include "Cif2Geometry.h"
#include "FileView.h"
#include "Text.h"
#include "TextButton.h"
#include "Window.h"

FileView::FileView(Scene *prev, int start) : Scene(prev)
{
	_manager = Window::fileManager();
	_manager->setFileView(this);
	_start = start;
}

FileView::~FileView()
{
	_manager->setFileView(nullptr);
	deleteObjects();
}

void FileView::setup()
{
	{
		Text *text = new Text("Vagabond file system");
		setCentre(text, 0.5, 0.1);
		addObject(text);
	}
	
	refreshFiles();
}

void FileView::buttonPressed(std::string tag, Button *button)
{
	Scene::buttonPressed(tag, button);

	std::string file_prefix = "file_";
	if (tag.rfind(file_prefix, 0) != std::string::npos)
	{
		std::string filename = tag.substr(file_prefix.length(), 
		                                  std::string::npos);

		std::cout << "Here " << "loading " << filename << std::endl;
		Cif2Geometry geom = Cif2Geometry(filename);
		geom.parse();
		
		if (geom.atomCount() > 0)
		{
			Display *display = new Display(this);
			display->loadAtoms(geom.atoms());
			display->show();
		}

	}
}

void FileView::refreshFiles()
{
	loadFilesFrom(_start, 10);
}

void FileView::loadFilesFrom(int start, int num)
{
	for (size_t i = 0; i < _temps.size(); i++)
	{
		removeObject(_temps[i]);
		delete _temps[i];
	}
	_temps.clear();

	int fileCount = _manager->fileCount();
	float pos = 0.3;

	for (size_t i = start; i < start + num && i < fileCount; i++)
	{
		std::string filename = _manager->filename(i);

		TextButton *button = new TextButton(filename, this);
		button->setReturnTag("file_" + filename);
		button->resize(1.0);
		setLeft(button, 0.2, pos);
		pos += 0.06;
		addObject(button);
		_temps.push_back(button);
	}
}
