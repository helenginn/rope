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
#include "FileView.h"
#include "FileLine.h"

#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TextButton.h>

#include <vagabond/core/CifFile.h>
#include <vagabond/core/FileManager.h>
#include <vagabond/core/AtomContent.h>
#include <vagabond/core/Environment.h>

FileView::FileView(Scene *prev) : ListView(prev)
{
	_manager = Environment::fileManager();
	_manager->setFileView(this);
}

FileView::FileView(FileViewResponder *prev, bool choose) : ListView(prev)
{
	_responder = prev;
	_manager = Environment::fileManager();
	_manager->setFileView(this);
}

FileView::~FileView()
{
	_manager->setFileView(nullptr);
}

void FileView::filterForTypes(File::Type type)
{
	_manager->setFilterType(type);
}

void FileView::setup()
{
	{
		Text *text = new Text("Vagabond file system");
		text->setCentre(0.5, 0.1);
		addObject(text);
	}
	
	ListView::setup();
}

void FileView::handleFileWithoutChoice(std::string filename)
{
	File *file = File::loadUnknown(filename);
	CifFile::Type type = file->cursoryLook();

	if (type & CifFile::CompAtoms || type & CifFile::MacroAtoms)
	{
		if (file->atomCount() > 0)
		{
			Display *display = new Display(this);
			display->loadAtoms(file->atoms());
			display->show();
		}
	}
	else if (type & CifFile::Reflections)
	{
		Display *display = new Display(this);
		display->loadDiffraction(file->diffractionData());
		display->show();
	}

	delete file;
}

void FileView::returnToResponder(std::string filename)
{
	_responder->fileChosen(filename);
	Scene::back();
}

void FileView::buttonPressed(std::string tag, Button *button)
{
	Scene::buttonPressed(tag, button);

	std::string file_prefix = "file_";
	if (tag.rfind(file_prefix, 0) != std::string::npos)
	{
		std::string filename = tag.substr(file_prefix.length(), 
		                                  std::string::npos);

		if (_responder == nullptr)
		{
			handleFileWithoutChoice(filename);
		}
		else
		{
			returnToResponder(filename);
		}
	}
	
	ListView::buttonPressed(tag, button);
}

Renderable *FileView::getLine(int i)
{
	std::string filename = _manager->filtered(i);

	FileLine *line = new FileLine(this, filename);
	return line;
}

size_t FileView::lineCount()
{
	int fileCount = _manager->filteredCount();
	return fileCount;
}

void FileView::filesChanged()
{
	refreshFiles();
}
