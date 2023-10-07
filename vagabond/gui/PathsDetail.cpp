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

#include "PathsDetail.h"
#include "RouteValidator.h"
#include <vagabond/core/Path.h>

PathsDetail::PathsDetail(Scene *prev, Path *p) : 
Scene(prev),
AddObject<Path>(prev, p)
{
	calculateMetrics();

}

void PathsDetail::calculateMetrics()
{
	PlausibleRoute *pr = _obj.toRoute();
	pr->setup();
	pr->submitJobAndRetrieve(0);

	RouteValidator rv(*pr);

	_linearity = rv.linearityRatio();
	_valid = rv.validate();
	_rmsd = rv.rmsd();
	_obj.cleanupRoute();
}

void PathsDetail::setup()
{
	addTitle("Path details");
	AddObject::setup();

	Text *t = new Text(_obj.desc());
	t->setCentre(0.5, 0.15);
	t->squishToWidth(0.8);
	addObject(t);
	
	float top = 0.3;
	
	{
		Text *t = new Text("Route validity:");
		t->setLeft(0.2, top);
		addObject(t);
	}
	
	{
		std::string str = _valid ? "valid" : "not valid";
		Text *t = new Text(str);
		t->setRight(0.8, top);
		addObject(t);
		
		if (!_valid)
		{
			top += 0.06;
			std::string str = ("Deviation: " + std::to_string(_rmsd) 
			                   + " Angstroms");
			Text *t = new Text(str);
			t->addAltTag("Threshold of validity would be 0.02 Angstroms");
			t->setRight(0.8, top);
			addObject(t);
		}
	}
	
	top += 0.1;
	
	{
		Text *t = new Text("Measure of linearity:");
		t->setLeft(0.2, top);
		addObject(t);
	}
	
	{
		std::string str = std::to_string(_linearity);
		Text *t = new Text(str);
		t->addAltTag("Well-refined paths usually have linearity above 0.9");
		t->setRight(0.8, top);
		addObject(t);
	}
}

void PathsDetail::buttonPressed(std::string tag, Button *button)
{
	if (tag == "delete" && _deleteAllowed)
	{
		Environment::purgePath(_obj);
		back();
	}

	AddObject::buttonPressed(tag, button);
}
