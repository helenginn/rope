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

#include "MetadataView.h"
#include "ChooseHeader.h"
#include "TableView.h"

#include <vagabond/utils/FileReader.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Environment.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Menu.h>

MetadataView::MetadataView(Scene *prev, Metadata *md) : Scene(prev)
{
	_md = md;

}

MetadataView::~MetadataView()
{
	if (!_md->isMaster())
	{
		delete _md;
		_md = nullptr;
	}
}

void MetadataView::setup()
{
	addTitle("Metadata summary - " + _md->source());

	{
		setExportHandler(this);
		TextButton *t = new TextButton("View data", this);
		t->setRight(0.9, 0.1);
		t->setReturnTag("view");
		addObject(t);
	}
	{
		Text *t = new Text("Number of entries");
		t->setLeft(0.2, 0.3);
		addObject(t);
	}
	{
		std::string num = i_to_str(_md->entryCount());
		Text *t = new Text(num);
		t->setLeft(0.8, 0.3);
		addObject(t);
	}

	{
		std::string str = "View " + i_to_str(_md->headerCount()) + " categories";
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
	
	if (_md != Environment::metadata())
	{
		TextButton *t = new TextButton("Add to database", this);
		t->setCentre(0.5, 0.8);
		t->setReturnTag("add");
		addObject(t);
	}
}

void MetadataView::supplyCSV(std::string indicator)
{
	_csv = _md->asCSV();
}

void MetadataView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "add")
	{
		Metadata &master = *Environment::metadata();
		master += *_md;
		back();
	}
	else if (tag == "headers")
	{
		ChooseHeader *ch = new ChooseHeader(this, false);
		std::set<std::string> headers = _md->headers();
		
		if (_md == Environment::metadata())
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
		if (_md->instanceEntryCount() > 1)
		{
			m->addOption("Individual molecules", "instances");
		}
		if (_md->modelEntryCount() > 1)
		{
			m->addOption("Whole models", "models");
		}
		m->setup(c.x, c.y);
		setModal(m);
	}
	else if (tag == "view_instances")
	{
		std::string title = "Metadata table - " + _md->source();
		TableView *view = new TableView(this, _md->asInstanceData(), title);
		view->show();
	}
	else if (tag == "view_models")
	{
		std::string title = "Metadata table - " + _md->source();
		TableView *view = new TableView(this, _md->asModelData(), title);
		view->show();
	}
	
	ExportsCSV::buttonPressed(tag, button);
	Scene::buttonPressed(tag, button);
}

void MetadataView::sendObject(std::string tag, void *object)
{
	std::cout << "Sent: " << tag << std::endl;
	std::string key = Button::tagEnd(tag, "__del_");
	
	std::cout << "Key: " << key << std::endl;
	if (key.length())
	{
		_md->purgeKey(key);
		std::set<std::string> headers = _md->headers();
		ChooseHeader *ch = static_cast<ChooseHeader *>(object);
		ch->setHeaders(headers);
		ch->refresh();
	}

}
