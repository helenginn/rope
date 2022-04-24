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

void ModelMenu::setup()
{
	addTitle("Model menu");
	
	ListView::setup();
}

size_t ModelMenu::lineCount()
{
	return 1 + _manager->modelCount();
}

Renderable *ModelMenu::getLine(int i)
{
	if (i == _manager->modelCount())
	{
		TextButton *t = new TextButton("Add model...", this);
		t->setReturnTag("add");
		return t;
	}
	
	Model &m = _manager->model(i);
	TextButton *t = new TextButton(m.name(), this);
	t->setReturnTag("model_" + m.name());
	return t;
}

void ModelMenu::buttonPressed(std::string tag, Button *button)
{
	ListView::buttonPressed(tag, button);

	if (tag == "add")
	{
		AddModel *addModel = new AddModel(this);
		addModel->show();
	}
}

void ModelMenu::modelsChanged()
{
	refreshFiles();
}
