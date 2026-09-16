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
#include <fstream>
#include "AddModel.h"
#include "Display.h"
#include "DisplayUnit.h"
#include "GuiAtom.h"
#include <vagabond/core/grids/ArbitraryMap.h>
#include <vagabond/gui/Toolkit.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/core/Environment.h>
#include "elements/Parallelepiped.h"

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
		
		Display *d = new Display(this);
		DisplayUnit *unit = new DisplayUnit(d);
		unit->setOwnsAtoms();
		unit->loadModel(model);
		d->tieButton();
		std::ifstream file;
		file.open("dhelix_hedgehog.csv");
		if (file.is_open())
		{
			std::string line {};
			std::vector<glm::vec4> tests {};
			glm::vec4 readPos {};
			float minVal = {10};
			float maxVal = {-10};
			while (getline(file, line))
			{
				std::istringstream iss(line);
				std::string lineStream;
				std::vector<float> xyz {};
				while (getline(iss, lineStream, ','))
				{
					xyz.push_back(std::stof(lineStream)); // convert to double
				}
				readPos.x = xyz[0];
				readPos.y = xyz[1];
				readPos.z = xyz[2];
				readPos.w = xyz[3];
				if (xyz[3] < minVal)
					minVal = xyz[3];
				else if (xyz[3] > maxVal)
					maxVal = xyz[3];
				tests.push_back(readPos);
			}
			glm::vec3 startPos {};
			for (auto atoms : model->currentAtoms()->atomVector())
			{
				if (atoms->chain() == "C")
				{
					startPos = atoms->derivedPosition();
					break;
				}
			}
			glm::vec3 max (1.f,0.f,0.f);
			glm::vec3 min (0.f,0.f,1.f);
			float capped = minVal +(maxVal- minVal)/16;
			for (auto const &vectors : tests)
			{
				Parallelepiped *para1 = new Parallelepiped();
				d->addObject(para1);
				para1->addTrueParallelepiped(startPos, glm::vec3(vectors), 0.6, 0.9);
				{
					float pos = vectors.w - minVal;
					glm::vec3 colour{};
					if (pos <= capped - minVal)
						colour = (pos / capped) * max + (1 - pos / capped) * min;
					else
						colour = max;
					para1->setColour(colour.x, colour.y, colour.z);
				}
				para1->setAlpha(0.6f);
				para1->forceRender();
			}
		}
		ArbitraryMap *map = new ArbitraryMap(model->dataFile());
		if (map->nn() > 0)
		{
			unit->densityFromMap(map);
		}
		
		d->addDisplayUnit(unit);
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
