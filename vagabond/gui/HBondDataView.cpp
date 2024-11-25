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
#include "HBondDataView.h"
#include "ChooseHeader.h"
#include "TableView.h"

#include <vagabond/utils/FileReader.h>
#include <vagabond/core/HBondData.h>
#include <vagabond/core/Database.h>
#include <vagabond/core/Environment.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Menu.h>


HBondDataView::HBondDataView(Scene *prev, HBondData *hbd) : Scene(prev)
{
	_hbd = hbd;
}

HBondDataView::~HBondDataView()
{
	if (!_hbd->isMaster())
	{
		delete _hbd;
		_hbd = nullptr;
	}
}

void HBondDataView::setup()
{
	addTitle("H-bonds summary -  List of sets" + _hbd->source());

	{
		TextButton *t = new TextButton("View data", this);
		t->setRight(0.9, 0.1);
		t->setReturnTag("view");
		addObject(t);
	}
	{
		// this is 
		Text *t = new Text("Number of h-bond sets");
		t->setLeft(0.2, 0.3);
		addObject(t);
	}
	{
		std::string str = "View " + i_to_str(_hbd->headerCount()) + " categories";
		Text *t = new Text(str);
		t->setLeft(0.2, 0.4);
		addObject(t);
	}
	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("headers");
		t->setCentre(0.8, 0.4);
		addObject(t);
	}
	

	if (_hbd->hBondEntryCount() >= 1)
	{
		TextButton *t = new TextButton("Add H-bond data", this);
		t->setCentre(0.5, 0.8);
		t->setReturnTag("add_hbond");
		addObject(t);
	}
}


void HBondDataView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "add_hbond")
	{
		std::cout << "Adding to hbond data file: " << _hbd->source() << std::endl;		
		HBondData &master = *Environment::hBondData();
		master += *_hbd;
		back();
	}
	else if (tag == "headers")
	{
		ChooseHeader *ch = new ChooseHeader(this, false);
		std::set<std::string> headers = _hbd->headers();
		
		if (_hbd == Environment::hBondData())
		{
			ch->setCanDelete(true);
			ch->setResponder(this);
		}

		ch->setHeaders(headers);
		ch->show();
	}
	else if (tag == "view")
	{
		glm::vec2 c = button->xy();
		Menu *m = new Menu(this, this, "view");
		if (_hbd->hBondEntryCount() >= 1)
		{
			m->addOption("H-bonds", "hbonds");
		}
		m->setup(c.x, c.y);
		setModal(m);
	}
	else if (tag == "view_hbonds")
	{
		std::string title = "H-bonds table - " + _hbd->source();
		TableView *view = new TableView(this, _hbd->asHBondData(), title);
		view->show();
	}

	Scene::buttonPressed(tag, button);
}

void HBondDataView::sendObject(std::string tag, void *object)
{
	std::cout << "Sent: " << tag << std::endl;
	std::string key = Button::tagEnd(tag, "__del_");
	
	std::cout << "Key: " << key << std::endl;
	if (key.length())
	{
		_hbd->purgeKey(key);
		std::set<std::string> headers = _hbd->headers();
		ChooseHeader *ch = static_cast<ChooseHeader *>(object);
		ch->setHeaders(headers);
		ch->refresh();
	}

}
