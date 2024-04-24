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

#ifndef __vagabond__ListInstancesView__
#define __vagabond__ListInstancesView__

#include <vagabond/gui/elements/ListView.h>
#include <map>

class ChooseHeader;

class ListInstancesView : public ListView, public Responder<ChooseHeader>,
public HasResponder<Responder<ListInstancesView>>
{
public:
	ListInstancesView(Scene *prev);

	void setFromList(const std::vector<std::string> &list)
	{
		_fromList = list;
	}

	void setToList(const std::vector<std::string> &list)
	{
		_toList = list;
	}
	
	virtual void setup();
	
	const std::string &selected() const
	{
		return _selected;
	}

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);
	
	struct ActivePair
	{
		bool active;
		std::string id;
	};
	
	typedef std::map<std::string, ActivePair> Map;
	
	void setMap(const Map &map)
	{
		_fromTo = map;
	}
	
	virtual void sendObject(std::string tag, void *object);

	virtual void buttonPressed(std::string tag, Button *button);
private:
	std::vector<std::string> _fromList;
	std::vector<std::string> _toList;

	Map _fromTo;
	
	std::string _selected;
};

#endif
