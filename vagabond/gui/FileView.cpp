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
#include <vagabond/core/CifFile.h>
#include "FileView.h"
#include "ImageButton.h"
#include "FileLine.h"
#include "Text.h"
#include "TextButton.h"
#include "Window.h"

#define LINES_PER_PAGE 8

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
		text->setCentre(0.5, 0.1);
		addObject(text);
	}
	
	refreshFiles();
}

void FileView::buttonPressed(std::string tag, Button *button)
{
	Scene::buttonPressed(tag, button);
	std::cout << tag << std::endl;

	std::string file_prefix = "file_";
	if (tag.rfind(file_prefix, 0) != std::string::npos)
	{
		std::string filename = tag.substr(file_prefix.length(), 
		                                  std::string::npos);

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
	
	if (tag == "scroll_forward")
	{
		_start += LINES_PER_PAGE;
		refreshFiles();
	}
	if (tag == "scroll_back")
	{
		_start -= LINES_PER_PAGE;
		if (_start < 0)
		{
			_start = 0;
		}
		refreshFiles();
	}
}

void FileView::refreshFiles()
{
	loadFilesFrom(_start, LINES_PER_PAGE);
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
	int npages = ceil(fileCount / LINES_PER_PAGE);
	double pos = 0.3;

	for (size_t i = start; i < start + num && i < fileCount; i++)
	{
		std::string filename = _manager->filename(i);
		
		FileLine *line = new FileLine(this, filename);
		line->setLeft(0.2, pos);
		pos += 0.06;
		addObject(line);
		_temps.push_back(line);
	}
	
	if (npages > 1)
	{
		int mypage = _start / LINES_PER_PAGE;
		std::ostringstream ss;
		ss << "(" << mypage + 1 << " / " << npages + 1 << ")";
		Text *pageNo = new Text(ss.str());
		pageNo->setCentre(0.5, 0.8);
		addObject(pageNo);
		_temps.push_back(pageNo);

	}
	
	if (start > 0)
	{
		scrollBackButton();
	}
	
	if (fileCount > start + num)
	{
		scrollForwardButton();
	}
}

void FileView::scrollBackButton()
{
	ImageButton *bb = new ImageButton("assets/images/arrow.png", this);
	bb->rescale(0.1, 0.05);
	glm::mat3x3 cw = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(90.),
	                                         glm::vec3(0., 0., -1.)));
	bb->rotateRoundCentre(cw);
	bb->setCentre(0.1, 0.8);
	bb->setReturnTag("scroll_back");
	addObject(bb);
	_temps.push_back(bb);
}

void FileView::scrollForwardButton()
{
	ImageButton *fb = new ImageButton("assets/images/arrow.png", this);
	fb->rescale(0.1, 0.05);
	glm::mat3x3 ccw = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(-90.),
	                                          glm::vec3(0., 0., -1.)));
	fb->rotateRoundCentre(ccw);
	fb->setCentre(0.9, 0.8);
	fb->setReturnTag("scroll_forward");
	addObject(fb);
	_temps.push_back(fb);
}
