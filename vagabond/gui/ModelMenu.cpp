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
#include "Display.h"
#include "GuiAtom.h"
#include "ArbitraryMap.h"
#include <vagabond/gui/Toolkit.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/core/Environment.h>

ModelMenu::ModelMenu(Scene *prev) : ListView(prev)
{
	_manager = Environment::modelManager();
	_manager->Manager::setResponder(this);
}

ModelMenu::~ModelMenu()
{

}

void ModelMenu::addAutomodelButton()
{
	if (Environment::entityCount() > 0)
	{
		Toolkit *tk = new Toolkit(this);
		_temps.push_back(tk);
		addObject(tk);
	}
}

void ModelMenu::setup()
{
	addTitle("Model menu");
	ListView::setup();
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
	Box *b = new Box();
	{
		TextButton *t = new TextButton(m.name(), this);
		t->setReturnTag("model_" + m.name());
		t->setLeft(0.0, 0.0);
		b->addObject(t);
	}

	{
		ImageButton *t = new ImageButton("assets/images/eye.png", 
		                                      this);
		t->setReturnTag("refine_" + m.name());
		t->resize(0.06);
		t->setRight(0.6, 0.0);
		b->addObject(t);
	}

	return b;
}

void ModelMenu::buttonPressed(std::string tag, Button *button)
{
	ListView::buttonPressed(tag, button);

	if (tag == "add")
	{
		AddModel *addModel = new AddModel(this);
		addModel->show();
		return;
	}

	std::string name = Button::tagEnd(tag, "model_");
	if (name.length() > 0)
	{
		Model *model = _manager->model(name);
		
		AddModel *addModel = new AddModel(this, model);
		addModel->show();
		return;
	}

	name = Button::tagEnd(tag, "refine_");
	if (name.length() > 0)
	{
		std::cout << "Refine model" << std::endl;
		refineModel(name);
	}
}

void ModelMenu::refineModel(std::string name)
{
	try
	{
		Model *model = _manager->model(name);
		model->load();
		_currModel = model;
		AtomContent *atoms = model->currentAtoms();

		Display *d = new Display(this);
		d->loadAtoms(atoms);
		d->setOwnsAtoms(false);
		d->setMultiBondMode(true);
		d->guiAtoms()->setDisableRibbon(false);
		ArbitraryMap *map = new ArbitraryMap(model->dataFile());
		d->densityFromMap(map);
		d->show();
	}
	catch (std::runtime_error &err)
	{
		BadChoice *bc = new BadChoice(this, err.what());
		setModal(bc);
	}
}

void ModelMenu::refresh()
{
	ListView::refresh();
	
	if (_currModel != nullptr)
	{
		_currModel->unload();
		_currModel = nullptr;
	}

	addAutomodelButton();
}

void ModelMenu::respond()
{
	refreshNextRender();
}
