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

#include "AxesMenu.h"
#include <vagabond/c4x/Angular.h>
#include <vagabond/core/HasMetadata.h>
#include <vagabond/gui/elements/Choice.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextButton.h>

AxesMenu::AxesMenu(Scene *prev) : ListView(prev)
{

}

void AxesMenu::setup()
{
	addTitle("SVD axes");
	
	{
		setExportHandler(this);
		TextButton *t = new TextButton("Export", this);
		t->setRight(0.9, 0.1);
		t->setReturnTag("export");
		addObject(t);
	}
	
	{
		Text *t = new Text("Display");
		t->setRight(0.2, 0.2);
		addObject(t);
		
	}
	
	{
		Text *t = new Text("Number");
		t->setLeft(0.23, 0.2);
		addObject(t);
		
	}
	
	{
		Text *t = new Text("Weight");
		t->setRight(0.85, 0.2);
		addObject(t);
		
	}

	ListView::setup();
}

size_t AxesMenu::lineCount()
{
	return _cluster->rows();
}

Renderable *AxesMenu::getLine(int i)
{
	Box *b = new Box();
	
	bool found = false;
	for (size_t j = 0; j < _cluster->displayableDimensions(); j++)
	{
		if (_cluster->axis(j) == i)
		{
			found = true;
			break;
		}
	}
	
	{
		std::string text = "axis " + i_to_str(i);
		ChoiceText *t = new ChoiceText(text, this, this);

		t->setLeft(0, 0);
		b->addObject(t);
		
		t->click();
		if (!found)
		{
			t->unclick();
		}

		t->setReturnTag("axis_" + i_to_str(i));
	}
	
	{
		float weight = _cluster->weight(i);
		std::string text = f_to_str(weight, 2);
		Text *t = new Text(text);
		t->setRight(0.65, 0);
		b->addObject(t);
	}
	
	{
		ImageButton *fb = ImageButton::arrow(-90., this);
		fb->setLeft(0.7, 0.0);
		fb->setReturnTag("export_" + i_to_str(i));
		b->addObject(fb);
	}
	
	return b;
}

void AxesMenu::supplySingleAxis(int i)
{
	_csv = "";

	std::vector<ResidueTorsion> list = _cluster->dataGroup()->headers();
	MetadataGroup::Array vals = _cluster->rawVector(i);
	_csv += "torsion_id,torsion(degrees)\n";
	
	for (size_t i = 0; i < list.size(); i++)
	{
		_csv += list[i].desc() + ", " + std::to_string(vals[i]);
		_csv += "\n";
	}

}

void AxesMenu::supplyMainPlot()
{
	_csv = "";
	MetadataGroup *mg = _cluster->dataGroup();
	
	for (size_t i = 0; i < mg->objectCount(); i++)
	{
		HasMetadata *hm = mg->object(i);
		_csv += hm->id() + ",";
		
		for (size_t j = 0; j < _cluster->columns(); j++)
		{
			_csv += f_to_str(_cluster->value(i, j), 3) + ",";
		}
		
		_csv.pop_back();
		_csv += "\n";
	}

}

void AxesMenu::supplyCSV(std::string indicator)
{
	if (indicator == "")
	{
		supplyMainPlot();
	}
	else
	{
		int axis = atoi(indicator.c_str());
		supplySingleAxis(axis);
	}
}

void AxesMenu::buttonPressed(std::string tag, Button *button)
{
	std::string end = Choice::tagEnd(tag, "axis_");
	
	if (end.length())
	{
		int axis = atoi(end.c_str());
		_cluster->changeLastAxis(axis);
		refresh();
	}

	ListView::buttonPressed(tag, button);
	ExportsCSV::buttonPressed(tag, button);
}

