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

#ifndef __vagabond__PathInterfaceSetupView__
#define __vagabond__PathInterfaceSetupView__

#include <vagabond/gui/elements/ListView.h>
#include <map>

class Instance;
class Interface;
class Model;
class TickList;

class PathInterfaceSetupView : public ListView, public Responder<TickList>
{
public:
	PathInterfaceSetupView(Scene *scene, Model *from, Model *to);
	~PathInterfaceSetupView();
	
	void setMap(const std::map<std::string, std::string> &map)
	{
		_map = map;
		_list.clear();
		for (auto it = _map.begin(); it != _map.end(); it++)
		{
			_list.push_back(it->first);
		}
	}

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

	virtual void setup();
	
	virtual void sendObject(std::string tag, void *object);
private:
	void precalculateInterfaces();
	void preparePath();

	std::vector<std::string> _list;
	std::map<std::string, std::string> _map;
	
	struct Group
	{
		Instance *left;
		Instance *right;
		std::string desc;
		Interface *face;
		std::vector<std::string> chosen;
	};
	
	typedef std::pair<std::string, std::string> StringPair;

	std::vector<StringPair> sanitiseInteractions(Group &group);
	std::vector<Group> _interfaces;
	
	Group *_active = nullptr;
	
	Model *_from = nullptr;
	Model *_to = nullptr;
};

#endif
