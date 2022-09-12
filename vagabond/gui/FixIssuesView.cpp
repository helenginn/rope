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
#include <vagabond/gui/ResolveIssues.h>
#include <vagabond/gui/elements/Choice.h>
#include <vagabond/gui/elements/TextButton.h>

FixIssuesView::FixIssuesView(Scene *prev, Entity *ent) : Scene(prev)
{
	_entity = ent;
}

FixIssuesView::~FixIssuesView()
{
	if (_worker != nullptr)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}

	if (_fixer != nullptr)
	{
		delete _fixer;
		_fixer = nullptr;
	}

}

void FixIssuesView::addOption(std::string name, std::string tag, float top,
                              bool ticked)
{
	ChoiceText *opt = new ChoiceText(name, this, this);
	opt->setLeft(0.2, top);
	opt->setReturnTag(tag);
	addObject(opt);
	ticked ? opt->tick() : opt->untick();
	_options.push_back(opt);

}

void FixIssuesView::setup()
{
	addTitle("Fix issues in " + _entity->name());
	
	if (_entity->moleculeCount() == 0)
	{
		Text *t = new Text("No molecules for entity yet");
		t->setLeft(0.15, 0.2);
		addObject(t);
		return;
	}
	else
	{
		Molecule *m = _entity->molecules()[0];
		_molecule = m;
	}
	
	float inc = 0.06;
	float top = 0.3;
	addOption("Fix phenylalanines", "phenylalanine", top, true);
	top += inc;
	addOption("Fix tyrosines", "tyrosine", top, true);
	top += inc;
	addOption("Fix aspartate", "aspartate", top, true);
	top += inc;
	addOption("Fix glutamate", "glutamate", top, true);
	top += inc;
	addOption("Fix arginine", "arginine", top, true);
	top += inc;
	addOption("Fix asparagine", "asparagine", top, false);
	top += inc;
	addOption("Fix glutamine", "glutamine", top, false);
	top += inc;
	addOption("Fix peptide flips", "peptide_flips", top, false);
	
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

	for (ChoiceText *ch : _options)
	{
		bool ticked = ch->ticked();
		if (!ticked)
		{
			continue;
		}

		std::string tag = ch->tag();

		if (tag == "phenylalanine")
		{
			ret = (FixIssues::Options)(ret | FixIssues::FixPhenylalanine);
		}
		else if (tag == "tyrosine")
		{
			ret = (FixIssues::Options)(ret | FixIssues::FixTyrosine);
		}
		else if (tag == "aspartate")
		{
			ret = (FixIssues::Options)(ret | FixIssues::FixAspartate);
		}
		else if (tag == "glutamate")
		{
			ret = (FixIssues::Options)(ret | FixIssues::FixGlutamate);
		}
		else if (tag == "asparagine")
		{
			ret = (FixIssues::Options)(ret | FixIssues::FixAsparagine);
		}
		else if (tag == "arginine")
		{
			ret = (FixIssues::Options)(ret | FixIssues::FixArginine);
		}
		else if (tag == "peptide_flips")
		{
			ret = (FixIssues::Options)(ret | FixIssues::FixPeptideFlips);
		}
	}

	return ret;
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
		
		_worker = new std::thread(&FixIssues::findIssues, _fixer);
//		_fixer->findIssues();
	}

	Scene::buttonPressed(tag, button);
}
