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

#include "unistd.h"
#include <fstream>

#include "ProjectMenu.h"
#include "FileNavi.h"
#include "VagWindow.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/cmd/Dictator.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/ChooseHeader.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/utils/FileReader.h>

ProjectMenu::ProjectMenu(Scene *prev) : Scene(prev)
{
	findExistingProjects();
}

void ProjectMenu::setup()
{
	addTitle("Project menu");
	hideBackButton();

	float top = 0.3;

	{
		Text *t = new Text("Choose existing project");
		t->setLeft(0.2, top);
		addObject(t);
	}

	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("choose_project");
		t->setCentre(0.8, top);
		addObject(t);
	}
	
	top += 0.08;

	{
		Text *t = new Text("or find project folder");
		t->setLeft(0.2, top);
		addObject(t);
	}

	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("choose_folder");
		t->setCentre(0.8, top);
		addObject(t);
	}
	
	addPathOptions();
}

void fileTextOrChoose(std::string &file, std::string other)
{
	if (file.length() == 0)
	{
		file = other;
	}
}

void ProjectMenu::addPathOptions()
{
	deleteTemps();

	if (_path.length() == 0)
	{
		return;
	}
	
	float top = 0.46;

	{
		Text *t = new Text(_path);
		t->resize(0.6);
		t->setLeft(0.2, top);
		addTempObject(t);
	}
	
	top += 0.08;
	
	{
		Text *t = new Text("Choose project name:");
		t->setLeft(0.2, top);
		addTempObject(t);
	}

	{
		fileTextOrChoose(_name, "Enter...");

		TextEntry *t = new TextEntry(_name, this);
		t->setReturnTag("enter_name");
		t->setRight(0.8, top);

		_nameEntry = t;
		addTempObject(t);
	}
	
	{
		TextButton *tb = new TextButton("Create", this);
		tb->setReturnTag("create");
		tb->setLeft(0.8, 0.8);
		addTempObject(tb);
	}
}

void ProjectMenu::doThings()
{
	if (_finish)
	{
		back();
		VagWindow::window()->prepareProgressView();
		return;
	}
}

void ProjectMenu::refresh()
{
	_navi = nullptr;
	_header = nullptr;
	addPathOptions();
}

void ProjectMenu::buttonPressed(std::string tag, Button *button)
{
	if (tag == "choose_folder")
	{
		FileNavi *fn = new FileNavi(this);
		_navi = fn;
		fn->setResponder(this);
		fn->show();
	}
	if (tag == "choose_project")
	{
		ChooseHeader *ch = new ChooseHeader(this, true);
		ch->setTitle("Choose project");
		ch->setHeaders(_names);
		ch->setCanDelete(true);
		ch->setResponder(this);
		_header = ch;
		ch->show();
	}
	else if (tag == "create")
	{
		_name = _nameEntry->scratch();
		createProject();
	}
}

void ProjectMenu::sendObject(std::string tag, void *object)
{
	std::cout << tag << std::endl;
	if (object == _navi)
	{
		if (_path != tag)
		{
			_name = "";
		}

		_path = tag;
		_navi = nullptr;
	}
	else if (object == _header)
	{
		_header = nullptr;
		
		std::string end = Button::tagEnd(tag, "__del_");
		
		if (end.length())
		{
			for (size_t i = 0; i < _projects.size(); i++)
			{
				std::cout << "end: " << end << std::endl;
				if (_projects[i].rfind(end, 0) == 0)
				{
					_projects.erase(_projects.begin() + i);
					_names.erase(end);
					writeProjects();
					return;
				}
			}
		}

		for (size_t i = 0; i < _projects.size(); i++)
		{
			std::cout << tag << " " << _projects[i] << std::endl;
			if (_projects[i].rfind(tag, 0) == 0)
			{
				size_t colon = _projects[i].find(":");
				_path = _projects[i].substr(colon + 1, std::string::npos);
				std::cout << "Path: " << _path << std::endl;
				goToProject();
				_finish = true;
				return;
			}
		}

	}
}

void ProjectMenu::goToProject()
{
	chdir(_path.c_str());
	
	VagWindow::dictator()->addArg("environment=rope.json");
	VagWindow::dictator()->addArg("get-files-native-app");

}

void ProjectMenu::createProject()
{
	if (_names.count(_name))
	{
		BadChoice *bc = new BadChoice(this, "Project of same name already exists.");
		setModal(bc);
		return;
	}
	
	std::string project = _name + ":" + _path;
	_projects.push_back(project);
	writeProjects();

	goToProject();
	back();
}

void ProjectMenu::findExistingProjects()
{
	_projects.clear();
	_names.clear();

	if (file_exists("projects.txt"))
	{
		std::string all = get_file_contents("projects.txt");
		_projects = split(all, '\n');
	}
	
	for (size_t i = 0; i < _projects.size(); i++)
	{
		if (_projects[i].length() == 0)
		{
			_projects.erase(_projects.begin() + i);
			i--;
		}
	}
	
	for (const std::string &proj : _projects)
	{
		size_t first = proj.find(":");
		_names.insert(proj.substr(0, first));
	}
}

void ProjectMenu::writeProjects()
{
	std::ofstream file;
	file.open("projects.txt");
	
	for (size_t i = 0; i < _projects.size(); i++)
	{
		if (_projects[i].length())
		{
			file << _projects[i] << std::endl;
		}
	}
	
	file.close();
}
