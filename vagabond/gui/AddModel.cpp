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

#include "AddModel.h"

#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextEntry.h>

#include <vagabond/core/Environment.h>
#include <vagabond/core/ModelManager.h>

AddModel::AddModel(Scene *prev) : Scene(prev)
{

}

AddModel::~AddModel()
{
	deleteObjects();
}

void AddModel::setup()
{
	addTitle("Model menu - Add model");

	{
		Text *t = new Text("Initialising file:");
		t->setLeft(0.2, 0.3);
		t->addAltTag("Choose best atomic model from other program\n"\
		             "Used for topology and initial atom positions");
		addObject(t);
	}
	{
		std::string file = _m.filename();
		fileTextOrChoose(file);

		TextButton *t = new TextButton(file, this);
		t->setReturnTag("choose_initial_file");
		t->setRight(0.8, 0.3);
		_initialFile = t;
		addObject(t);
	}

	{
		Text *t = new Text("Model name:");
		t->setLeft(0.2, 0.4);
		t->addAltTag("Unique identifier for model");
		addObject(t);
	}
	{
		std::string file = _m.name();
		fileTextOrChoose(file, "Enter...");

		TextEntry *t = new TextEntry(file, this);
		t->setReturnTag("enter_name");
		t->setRight(0.8, 0.4);
		_name = t;
		addObject(t);
	}

	{
		Text *t = new Text("Chain assignment:");
		t->setLeft(0.2, 0.5);
		addObject(t);
	}
	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("chain_assignment");
		t->setCentre(0.8, 0.5);
		addObject(t);
	}
	
	{
		TextButton *t = new TextButton("Cancel", this);
		t->setLeft(0.2, 0.8);
		t->setReturnTag("cancel");
		addObject(t);
	}
	{
		TextButton *t = new TextButton("Create", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("create");
		addObject(t);
	}

}

void AddModel::fileTextOrChoose(std::string &file, std::string other)
{
	if (file.length() == 0)
	{
		file = other;
	}
}

void AddModel::refreshInfo()
{
	{
		std::string text = _m.filename();
		fileTextOrChoose(text);
		_initialFile->setText(text);
	}
	{
		std::string text = _m.name();
		fileTextOrChoose(text, "Enter...");
		_name->setText(text);
	}
}

void AddModel::buttonPressed(std::string tag, Button *button)
{
	_lastTag = tag;
	if (tag == "choose_initial_file")
	{
		FileView *view = new FileView(this, true);
		view->show();
	}
	else if (tag == "enter_name")
	{
		_m.setName(_name->text());
		refreshInfo();
	}
	else if (tag == "chain_assignment")
	{
//		ChainAssignment *view = new ChainAssignment(this, true);
//		view->show();
	}
	else if (tag == "cancel")
	{
		back();
	}
	else if (tag == "create")
	{
		try
		{
			Environment::modelManager()->insertIfUnique(_m);
			back();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bad = new BadChoice(this, err.what());
			setModal(bad);
		}
		catch (...)
		{
			BadChoice *bad = new BadChoice(this, "wtf");
			setModal(bad);
		}
	}
	
	Scene::buttonPressed(tag, button);
}

void AddModel::fileChosen(std::string filename)
{
	if (_lastTag == "choose_initial_file")
	{
		_m.setFilename(filename);
	}
	refreshInfo();

}
