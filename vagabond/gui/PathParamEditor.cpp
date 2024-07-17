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

#include "vagabond/gui/elements/TextEntry.h"
#include "vagabond/gui/elements/TextButton.h"
#include "PathParamEditor.h"
#include <vagabond/core/paths/Route.h>
#include <vagabond/utils/FileReader.h>

PathParamEditor::PathParamEditor(Scene *prev, Route *route) : ListView(prev)
{
	_route = route;

}

void PathParamEditor::setup()
{
	addTitle("Parameter editor");
	
	TextButton *clear = new TextButton("Wipe", this);
	clear->setRight(0.9, 0.1);
	clear->setReturnJob
	([this]()
	 {
		for (int i = 0; i < _route->motionCount(); i++)
		{
			_route->motion(i).wp = WayPoints(_route->order(), 
			                                 _route->randomPerturb());
		}
		refresh();
	});
	addObject(clear);

	ListView::setup();
}

size_t PathParamEditor::lineCount()
{
	return _route->motionCount();
}

void PathParamEditor::recalculateTotals()
{
	_totals.resize(_route->motionCount());
	_totals[0] = _route->motion(0).workingAngle();
	
	int prev = 0;
	for (int i = 1; i < _route->motionCount(); i++)
	{
		if (_route->parameter(i) && _route->parameter(i)->coversMainChain())
		{
			_totals[i] = _totals[prev] + _route->motion(i).workingAngle();
			prev = i;
		}
	}
}

void PathParamEditor::refresh()
{
	recalculateTotals();
	ListView::refresh();
}

Renderable *PathParamEditor::getLine(int i)
{
	Motion &motion = _route->motion(i);
	Parameter *const &param = _route->parameter(i);

	Box *box = new Box();
	
	if (param)
	{
		Atom *atom = param->anAtom();
		std::string text = atom->code() + atom->residueId().str();
		Text *pText = new Text(text);
		pText->resize(0.6);
		pText->setLeft(0.0, 0.0);
		box->addObject(pText);
	}
	
	{
		std::string text = param ? param->desc() : "null";
		Text *pText = new Text(text);
		pText->resize(0.6);
		pText->setLeft(0.1, 0.0);
		box->addObject(pText);
	}
	
	{
		std::string working_angle = f_to_str(motion.workingAngle(), 1);
		Text *aText = new Text(working_angle);
		aText->resize(0.6);
		aText->setLeft(0.3, 0.0);
		box->addObject(aText);
	}
	
	if (param && param->coversMainChain())
	{
		std::string cumulative = f_to_str(_totals[i], 1);
		Text *aText = new Text(cumulative);
		aText->resize(0.6);
		aText->setLeft(0.4, 0.0);
		box->addObject(aText);
	}
	
	if (param && !param->isConstrained())
	{
		std::string flip = motion.flip ? "yes" : "no";
		TextButton *aText = new TextButton(flip, this);
		aText->setReturnJob
		([this, &motion]()
		 {
			motion.flip = !motion.flip;
			refresh();
		 });
		aText->resize(0.6);
		aText->setLeft(0.5, 0.0);
		box->addObject(aText);
	}
	
	if (param && !param->isConstrained())
	{
		float left = 0.6;
		for (int i = 0; i < 2; i++)
		{
			std::string order = f_to_str(motion.wp._amps[i], 1);
			TextEntry *te = new TextEntry(order, this);
			te->setReturnJob
			([te, &motion, i]()
			 {
				float first_order = atof(te->scratch().c_str());

				if (first_order == first_order && std::isfinite(first_order))
				{
					motion.wp._amps[i] = first_order;
				}
			});
			te->resize(0.6);
			te->setLeft(left, 0.0);
			left += 0.1;
			box->addObject(te);
		}
	}
	
	return box;
}
