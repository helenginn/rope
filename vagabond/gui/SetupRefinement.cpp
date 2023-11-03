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

#include <vagabond/core/Model.h>
#include <vagabond/core/Refinement.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/TextEntry.h>
#include "SetupRefinement.h"
#include "WatchRefinement.h"
#include "Axes.h"

SetupRefinement::SetupRefinement(Scene *prev, Model &model) 
: Scene(prev), AddObject(prev), _model(model)
{
	_obj.setModel(&model);

}

void SetupRefinement::setup()
{
	addTitle("Refinement: " + _model.name());
	
	bool can_start = true;
	std::string data = _model.dataFile();
	if (data == "")
	{
		data = "(not assigned)";
		can_start = false;
	}

	{
		Text *t = new Text("Data file:");
		t->setLeft(0.2, 0.3);
		addObject(t);
		

		Text *d = new Text(data);
		d->setRight(0.8, 0.3);
		addObject(d);
	}

	/*
	{
		Text *t = new Text("Protein conformational space:");
		t->setLeft(0.2, 0.4);
		addObject(t);
		

		TextButton *d = new TextButton("choose", this);
		d->setReturnTag("choose_space");
		d->setRight(0.8, 0.4);
		_space = d;
		addObject(d);
	}
	*/
	
	AddObject<Refinement>::setup();
}

void SetupRefinement::buttonPressed(std::string tag, Button *button)
{
	if (tag == "create")
	{
		try
		{
			checkValues();
			prepareRefinement();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bad = new BadChoice(this, err.what());
			setModal(bad);
		}
	}
	
	if (tag == "choose_space")
	{
		AskForText *aft = new AskForText(this, "Enter json filename", 
		                                 "json_filename", this);
		setModal(aft);
	}
	
	if (tag == "json_filename")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string text = te->scratch();
		_space->setText(text);
		_spaceName = text;
	}
	
	AddObject<Refinement>::buttonPressed(tag, button);
}

void SetupRefinement::checkValues()
{
	if (_model.dataFile() == "")
	{
		throw std::runtime_error("No data file provided by Model.");
	}
}

void SetupRefinement::prepareRefinement()
{
	_obj.loadDiffraction(_model.dataFile());
	_obj.setup();
	
	WatchRefinement *wr = new WatchRefinement(this, &_obj);
	wr->setSpaceFilename(_spaceName);
	wr->show();
	
	wr->start();
}
