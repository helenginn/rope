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
#include <vagabond/core/FixIssues.h>
#include <vagabond/gui/ResolveIssues.h>
#include <vagabond/gui/elements/TextButton.h>

FixIssuesView::FixIssuesView(Scene *prev, Entity *ent) : Scene(prev)
{
	_entity = ent;
}

FixIssuesView::~FixIssuesView()
{
	if (_fixer != nullptr)
	{
		delete _fixer;
		_fixer = nullptr;
	}

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
	
	{
		TextButton *t = new TextButton("Find issues", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("find_issues");
		addObject(t);
		return;
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
		_fixer->setOptions(FixIssues::FixPhenylalanine);
		_fixer->setModels(_entity->models());
		
		ResolveIssues *ri = new ResolveIssues(this, _fixer);
		_fixer->setResponder(ri);
		ri->show();
		
		_worker = new std::thread(&FixIssues::findIssues, _fixer);
//		_fixer->findIssues();
	}

	Scene::buttonPressed(tag, button);
}
