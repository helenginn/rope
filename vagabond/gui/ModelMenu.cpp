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

#include "ModelMenu.h"
#include "AddModel.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/Environment.h>

ModelMenu::ModelMenu(Scene *prev) : ListView(prev)
{
	_manager = Environment::modelManager();
	_manager->setResponder(this);
}

ModelMenu::~ModelMenu()
{
	deleteObjects();

}

void ModelMenu::addAutomodelButton()
{
	if (Environment::entityCount() > 0)
	{
		TextButton *t = new TextButton("Automodel...", this);
		t->setRight(0.9, 0.1);
		t->setReturnTag("automodel");
		_temps.push_back(t);
		addObject(t);
	}
}

void ModelMenu::setup()
{
	addTitle("Model menu");
	ListView::setup();

	addAutomodelButton();
	
}

size_t ModelMenu::lineCount()
{
	return 1 + _manager->objectCount();
}

Renderable *ModelMenu::getLine(int i)
{
	if (i == _manager->objectCount())
	{
		TextButton *t = new TextButton("Add model...", this);
		t->setReturnTag("add");
		return t;
	}
	
	Model &m = _manager->object(i);
	TextButton *t = new TextButton(m.name(), this);
	t->setReturnTag("model_" + m.name());
	return t;
}

void ModelMenu::buttonPressed(std::string tag, Button *button)
{
	ListView::buttonPressed(tag, button);

	if (tag == "automodel")
	{
		Environment::autoModel();
		return;
	}
	if (tag == "add")
	{
		AddModel *addModel = new AddModel(this);
		addModel->show();
		return;
	}

	std::string prefix = "model_";
	if (tag.rfind(prefix, 0) != std::string::npos)
	{
		std::string name = tag.substr(prefix.length(), std::string::npos);
		
		Model *model = _manager->model(name);
		
		AddModel *addModel = new AddModel(this, model);
		addModel->show();
		return;
	}
}

void ModelMenu::objectsChanged()
{
	refresh();
	
	addAutomodelButton();
}
