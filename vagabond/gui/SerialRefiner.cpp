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

#include "SerialRefiner.h"
#include "Display.h"
#include "GuiAtom.h"
#include <vagabond/core/Entity.h>
#include <vagabond/gui/elements/Text.h>
#include <sstream>

SerialRefiner::SerialRefiner(Scene *prev, Entity *entity) : Scene(prev)
{
	_entity = entity;
	_entity->setResponder(this);

}

SerialRefiner::~SerialRefiner()
{
	std::cout << "Deleting serial refiner" << std::endl;
	deleteObjects();
}

void SerialRefiner::setup()
{
	if (!_all)
	{
		addTitle("Refining " + i_to_str(_extra) + " models...");
		_entity->refineUnrefinedModels();
	}
	else if (_all)
	{
		addTitle("Refining all models...");
		_extra = _entity->moleculeCount();
		_entity->refineAllModels();
	}
}

void SerialRefiner::entityDone()
{

}

void SerialRefiner::doThings()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_newModel == false)
	{
		return;
	}

	_newModel = false;
	
	if (_model == nullptr)
	{
		return;
	}

	AtomGroup *atoms = _model->currentAtoms();

	if (atoms)
	{
		_display = new Display(this);
		_count += _model->moleculeCountForEntity(_entity->name());

		std::ostringstream ss;
		ss << "Refining " << _model->name() << " (" << _count << "/" <<
		_extra << ")" << std::endl;

		_display->addTitle(ss.str());
		_display->setControls(false);
		_display->setOwnsAtoms(false);
		_display->loadAtoms(atoms);
		_display->show();
		_display->guiAtoms()->setDisableRibbon(false);
		_display->guiAtoms()->setDisableBalls(true);
	}
	
	_model = nullptr;
}

void SerialRefiner::sendObject(std::string tag, void *object)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (tag == "model_done" && _display != nullptr)
	{
		_display->stop();
		_display->back();
		_display = nullptr;
	}

	if (tag != "model")
	{
		return;
	}
	
	_model = static_cast<Model *>(object);
	_newModel = true;
	
}

void SerialRefiner::respond()
{
	{
		Text *text = new Text("Done!", Font::Thin, true);
		text->setCentre(0.5, 0.5);
		addObject(text);
	}

}
