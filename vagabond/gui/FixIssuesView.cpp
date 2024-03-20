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

#include "FixIssuesView.h"
#include <vagabond/core/Entity.h>
#include <vagabond/utils/version.h>
#include <vagabond/gui/ResolveIssues.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>

FixIssuesView::FixIssuesView(Scene *prev, Entity *ent) : Scene(prev)
{
	if (!ent->hasSequence())
	{
		throw std::runtime_error("Entity for issue fixing does not contain "
		                         "protein");
	}
	_entity = ent;
}

FixIssuesView::~FixIssuesView()
{
	stop();

	if (_fixer != nullptr)
	{
		delete _fixer;
		_fixer = nullptr;
	}

}

void FixIssuesView::setup()
{
	addTitle("Fix issues in " + _entity->name());
	
	if (_entity->instanceCount() == 0)
	{
		Text *t = new Text("No instances for entity yet");
		t->setLeft(0.15, 0.2);
		addObject(t);
		return;
	}
	else
	{
		Polymer *m = static_cast<Polymer *>(_entity->instances()[0]);
		_molecule = m;
	}
	
	float inc = 0.06;
	float top = 0.2;
	_tickboxes = new TickBoxes(this, this);
	_tickboxes->addOption("Fix phenylalanines", "phenylalanine", true);
	_tickboxes->addOption("Fix tyrosines", "tyrosine", true);
	_tickboxes->addOption("Fix aspartate", "aspartate", true);
	_tickboxes->addOption("Fix glutamate", "glutamate", true);
	_tickboxes->addOption("Fix arginine", "arginine", true);
	_tickboxes->addOption("Fix asparagine", "asparagine", false);
	_tickboxes->addOption("Fix glutamine", "glutamine", false);
	_tickboxes->addOption("Fix histidine", "histidine", false);
	_tickboxes->addOption("Fix peptide flips", "peptide_flips", false);
	_tickboxes->setVertical(true);
	_tickboxes->setOneOnly(false);
	_tickboxes->arrange(0.2, 0.2, 1.0, 0.8);
	addObject(_tickboxes);
	
	{
		TextButton *t = new TextButton("Find issues", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("find_issues");
		addObject(t);
		return;
	}
}

FixIssues::Options FixIssuesView::options()
{
	FixIssues::Options ret = FixIssues::FixNone;

	if (_tickboxes->isTicked("phenylalanine"))
	{
		ret = (FixIssues::Options)(ret | FixIssues::FixPhenylalanine);
	}
	else if (_tickboxes->isTicked("tyrosine"))
	{
		ret = (FixIssues::Options)(ret | FixIssues::FixTyrosine);
	}
	else if (_tickboxes->isTicked("aspartate"))
	{
		ret = (FixIssues::Options)(ret | FixIssues::FixAspartate);
	}
	else if (_tickboxes->isTicked("glutamate"))
	{
		ret = (FixIssues::Options)(ret | FixIssues::FixGlutamate);
	}
	else if (_tickboxes->isTicked("asparagine"))
	{
		ret = (FixIssues::Options)(ret | FixIssues::FixAsparagine);
	}
	else if (_tickboxes->isTicked("arginine"))
	{
		ret = (FixIssues::Options)(ret | FixIssues::FixArginine);
	}
	else if (_tickboxes->isTicked("histidine"))
	{
		ret = (FixIssues::Options)(ret | FixIssues::FixHistidine);
	}
	else if (_tickboxes->isTicked("peptide_flips"))
	{
		ret = (FixIssues::Options)(ret | FixIssues::FixPeptideFlips);
	}

	return ret;
}

void FixIssuesView::stop()
{
	if (_worker != nullptr)
	{
		_fixer->stop();
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}
}

void FixIssuesView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "find_issues")
	{
		if (_fixer != nullptr)
		{
			delete _fixer;
			_fixer = nullptr;
		}
		
		_fixer = new FixIssues(_molecule);
		_fixer->setOptions(options());
		_fixer->setModels(_entity->models());
		
		ResolveIssues *ri = new ResolveIssues(this, _fixer);
		_fixer->setResponder(ri);
		ri->show();
		
		stop();
		_worker = new std::thread(&FixIssues::findIssues, _fixer);
	}
	
	if (tag == "back")
	{
		stop();
	}

	Scene::buttonPressed(tag, button);
}
