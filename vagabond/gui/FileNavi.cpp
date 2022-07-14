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

#include "FileNavi.h"
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextButton.h>
#include <stdlib.h>

FileNavi::FileNavi(Scene *prev) : ListView(prev)
{
	getPathContents();

}

void FileNavi::setup()
{
	addTitle("File navigator");
	
	{
		Text *t = new Text(_currentPath);
		t->resize(0.5);
		t->setCentre(0.5, 0.2);
		addObject(t);
		_pathName = t;
	}

	ListView::setup();
}

void FileNavi::buttonPressed(std::string tag, Button *button)
{
	if (button->returnObject() != this)
	{
		// ignore
	}
	else if (tag == "[choose]")
	{
		sendResponse(_currentPath, this);
		back();
		return;
	}
	else
	{
		std::string next = formNextPath(tag);
		getPathContents(next);
		refresh();
		return;
	}

	ListView::buttonPressed(tag, button);
}

void FileNavi::refresh()
{
	_pathName->setText(_currentPath);
	ListView::refresh();
}

std::string FileNavi::formNextPath(std::string end)
{
	if (end == "..")
	{
		std::string path = _currentPath;
		size_t pos = path.rfind("/");
		path = path.substr(0, pos);
		return path;
	}
	else
	{
		return (_currentPath + "/" + end);
	}
}

size_t FileNavi::lineCount()
{
	return _paths.size();
}

Renderable *FileNavi::getLine(int i)
{
	std::string path = _paths[i];
	
	if (path.rfind(_currentPath, 0) == 0)
	{
		size_t length = _currentPath.length();
		path = path.substr(length + 1, std::string::npos);
	}

	TextButton *tb = new TextButton(path, this);
	tb->setReturnTag(path);
	tb->setReturnObject(this);
	tb->setLeft(0.2, 0.0);
	
	if (!is_directory(_paths[i]) && i > 0)
	{
		tb->setInert(true);
		tb->setAlpha(-0.4);
	}

	return tb;

}

void FileNavi::getPathContents(std::string path)
{
	_start = 0;

	if (path.length() == 0)
	{
		char *home = getenv("HOME");
		path = std::string(home);
	}

	std::string pattern = path + "/*";
	
	_currentPath = path;
	std::vector<std::string> tmp = glob_pattern(pattern);

	_paths.clear();
	_paths.reserve(tmp.size() + 2);
	_paths.insert(_paths.begin(), "[choose]");
	_paths.insert(_paths.begin() + 1, "..");
	_paths.insert(_paths.end(), tmp.begin(), tmp.end());
}
