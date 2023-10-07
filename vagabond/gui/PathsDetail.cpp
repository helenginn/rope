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
#include <vagabond/core/RouteValidator.h>
#include "RouteExplorer.h"
#include <vagabond/core/Path.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Image.h>

PathsDetail::PathsDetail(Scene *prev, Path *p) : 
Scene(prev),
AddObject<Path>(prev, p)
{
//	PlausibleRoute *pr = _obj.toRoute();
//	pr->setup();

	calculateMetrics();

}

void PathsDetail::calculateMetrics()
{
	PlausibleRoute *pr = _obj.toRoute();
	pr->setup();

	RouteValidator rv(*pr);

	_linearity = rv.linearityRatio();
	_valid = rv.validate();
	_rmsd = rv.rmsd();
	_obj.cleanupRoute();
}

void PathsDetail::refresh()
{
	calculateMetrics();
	redraw();
}

void PathsDetail::redraw()
{
	deleteTemps();

	float top = 0.3;
	{
		std::string str = _valid ? "valid" : "not valid";
		Text *t = new Text(str);
		t->setRight(0.8, top);
		addTempObject(t);
		if (!_valid)
		{
			t->setColour(0.5, 0.0, 0.0);

			top += 0.06;
			std::string str = ("Deviation: " + std::to_string(_rmsd) 
			                   + " Angstroms");
			Text *t = new Text(str);
			t->addAltTag("Threshold of validity would be 0.02 Angstroms");
			t->setRight(0.8, top);
			addTempObject(t);

			Image *image = new Image("assets/images/sad_face.png");
			image->resize(0.04);
			image->setCentre(0.83, top);
			addTempObject(image);
		}
	}
	
	top = 0.5;
	
	{
		std::string str = std::to_string(_linearity);
		Text *t = new Text(str);
		t->addAltTag("Well-refined paths usually have linearity above 0.9");
		t->setRight(0.8, top);
		addTempObject(t);
		
		if (_linearity > 0.9 && _valid)
		{
			Image *image = new Image("assets/images/happy_face.png");
			image->resize(0.04);
			image->setCentre(0.83, top);
			addTempObject(image);
		}
	}
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
	
	top = 0.5;
	
	{
		Text *t = new Text("Measure of linearity:");
		t->setLeft(0.2, top);
		addObject(t);
	}
	
	top = 0.6;

	{
		Text *t = new Text("Note: path validation does not currently include any "\
		                   "estimation of\natomic clashes or remedy against them.");
		t->squishToWidth(0.6);
		t->setLeft(0.2, top);
		addObject(t);
	}

	{
		TextButton *t = new TextButton("View", this);
		t->setLeft(0.2, 0.8);
		t->setReturnTag("view");
		addObject(t);
	}
	
	redraw();
}

void PathsDetail::buttonPressed(std::string tag, Button *button)
{
	if (tag == "delete" && _deleteAllowed)
	{
		Environment::purgePath(_obj);
		back();
	}
	
	if (tag == "view")
	{
		PlausibleRoute *pr = _obj.toRoute();
		RouteExplorer *re = new RouteExplorer(this, pr);
		re->show();
	}

	AddObject::buttonPressed(tag, button);
}
