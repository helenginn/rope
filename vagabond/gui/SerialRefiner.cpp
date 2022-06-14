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
	_entity->setResponder(nullptr);
	deleteObjects();
}

void SerialRefiner::setup()
{
	addTitle("Refining " + i_to_str(_extra) + " models...");

	_entity->refineUnrefinedModels();
}

void SerialRefiner::entityDone()
{
	{
		Text *text = new Text("Done!");
		text->setCentre(0.5, 0.5);
		addObject(text);
	}

}

void SerialRefiner::setActiveAtoms(Model *model)
{
	if (_display != nullptr)
	{
		_display->stop();
		_display->back();
		_display = nullptr;
	}
	
	if (!model)
	{
		return;
	}

	AtomGroup *atoms = model->currentAtoms();

	if (atoms)
	{
		_display = new Display(this);
		_count += model->moleculeCountForEntity(_entity->name());

		std::ostringstream ss;
		ss << "Refining " << model->name() << " (" << _count << "/" <<
		_extra << ")" << std::endl;

		_display->setFutureTitle(ss.str());
		_display->setControls(false);
		_display->setOwnsAtoms(false);
		_display->loadAtoms(atoms);
		_display->queueToShow();
	}
}

void SerialRefiner::respond()
{

}
