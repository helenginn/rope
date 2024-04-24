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

#ifndef __vagabond__WholeModelPathSetupView__
#define __vagabond__WholeModelPathSetupView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/ListInstancesView.h>

class TextButton;
class ChooseHeader;

class WholeModelPathSetupView : public Scene, public Responder<ChooseHeader>,
public Responder<ListInstancesView>
{
public:
	WholeModelPathSetupView(Scene *prev);

	virtual void setup();
	virtual void refresh();

	void getStructure(bool from);
	virtual void sendObject(std::string tag, void *object);
	
	size_t chosenPairs();
private:

	TextButton *_fromButton = nullptr;
	TextButton *_toButton = nullptr;

	std::string _fromId;
	std::string _toId;
	bool _from = false;
	ListInstancesView::Map _map;
};

#endif
