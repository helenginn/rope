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

#ifndef __vagabond__ProjectMenu__
#define __vagabond__ProjectMenu__

#include <set>
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Responder.h>

class FileNavi;
class TextEntry;
class ChooseHeader;

class ProjectMenu : public Scene, public Responder<FileNavi>,
public Responder<ChooseHeader>
{
public:
	ProjectMenu(Scene *prev);
	
	virtual void setup();
	virtual void refresh();
	virtual void doThings();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	
	virtual void sendObject(std::string tag, void *object);
private:
	void goToProject();
	void addPathOptions();
	void createProject();
	void writeProjects();
	void findExistingProjects();

	std::vector<std::string> _projects;
	std::set<std::string> _names;
	std::string _path;
	std::string _name;
	TextEntry *_nameEntry;

	FileNavi *_navi = nullptr;
	ChooseHeader *_header = nullptr;
	
	bool _finish = false;
};

#endif
