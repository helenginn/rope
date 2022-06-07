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
#include "ChainAssignment.h"
#include "ModelMetadataView.h"

#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextEntry.h>

#include <vagabond/core/Environment.h>
#include <vagabond/core/ModelManager.h>

AddModel::AddModel(Scene *prev, Model *chosen) :
Scene(prev),
AddObject(prev, chosen),
FileViewResponder(prev)
{

}

AddModel::~AddModel()
{
	deleteObjects();
}

void AddModel::setup()
{
	std::string title = "Model menu - Add model";
	if (_existing)
	{
		title = "Model menu - edit model";
	}

	addTitle(title);
	float top = 0.3;
	float inc = 0.08;

	{
		Text *t = new Text("Initialising file:");
		t->setLeft(0.2, top);
		t->addAltTag("Choose best atomic model from other program\n"\
		             "Used for topology and initial atom positions");
		addObject(t);
	}
	{
		std::string file = _obj.filename();
		fileTextOrChoose(file);

		TextButton *t = new TextButton(file, this);
		t->setReturnTag("choose_initial_file");
		t->setRight(0.8, top);
		if (_existing) { t->setInert(); }
		_initialFile = t;
		addObject(t);
	}

	top += inc;
	{
		Text *t = new Text("Model name:");
		t->setLeft(0.2, top);
		t->addAltTag("Unique identifier for model");
		addObject(t);
	}
	{
		std::string file = _obj.name();
		fileTextOrChoose(file, "Enter...");

		TextEntry *t = new TextEntry(file, this);
		t->setReturnTag("enter_name");
		t->setRight(0.8, top);
		
		if (_existing) { t->setInert(); }

		_name = t;
		addObject(t);
	}

	top += inc;
	{
		Text *t = new Text("Chain assignment:");
		t->setLeft(0.2, top);
		addObject(t);
	}
	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("chain_assignment");
		t->setCentre(0.8, top);
		addObject(t);
	}

	top += inc;
	{
		Text *t = new Text("Data file:");
		t->setLeft(0.2, top);
		addObject(t);
	}
	{
		std::string file = _obj.dataFile();
		fileTextOrChoose(file);

		TextButton *t = new TextButton(file, this);
		t->setReturnTag("choose_data_file");
		t->setRight(0.8, top);
		_initialData = t;
		addObject(t);
	}

	top += inc;
	if (_existing)
	{
		{
			Text *t = new Text("Metadata");
			t->setLeft(0.2, top);
			addObject(t);
		}
		{
			ImageButton *t = ImageButton::arrow(-90., this);
			t->setReturnTag("metadata");
			t->setCentre(0.8, top);
			addObject(t);
		}
	}
	
	AddObject::setup();
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
		std::string text = _obj.dataFile();
		fileTextOrChoose(text);
		_initialData->setText(text);
	}
	{
		std::string text = _obj.filename();
		fileTextOrChoose(text);
		_initialFile->setText(text);
	}
	{
		std::string text = _obj.name();
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
		view->filterForTypes(File::MacroAtoms);
		view->show();
	}
	else if (tag == "choose_data_file")
	{
		FileView *view = new FileView(this, true);
		view->filterForTypes(File::Reflections);
		view->show();
	}
	else if (tag == "enter_name")
	{
		std::cout << "Name: " << _name->scratch() << std::endl;
		_obj.setName(_name->scratch());
		refreshInfo();
	}
	else if (tag == "chain_assignment")
	{
		try
		{
			ChainAssignment *view = new ChainAssignment(this, _obj);
			view->show();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bad = new BadChoice(this, err.what());
			setModal(bad);
		}
	}
	else if (tag == "metadata")
	{
		ModelMetadataView *mmv = new ModelMetadataView(this, _obj);
		mmv->show();
	}
	else if (tag == "create")
	{
		try
		{
			_obj.createMolecules();
			Environment::modelManager()->insertIfUnique(_obj);
			back();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bad = new BadChoice(this, err.what());
			setModal(bad);
		}
	}
	else if (tag == "delete" && _existing)
	{
		Environment::purgeModel(_obj.name());
		back();
	}
	else if (tag == "back" && _existing)
	{
		Environment::modelManager()->update(_obj);

	}
	
	AddObject::buttonPressed(tag, button);
}

void AddModel::fileChosen(std::string filename)
{
	if (_lastTag == "choose_initial_file")
	{
		_obj.setFilename(filename);
	}
	else if (_lastTag == "choose_data_file")
	{
		_obj.setDataFile(filename);
	}
	refreshInfo();

}
