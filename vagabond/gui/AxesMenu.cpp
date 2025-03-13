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
#include "TorsionAxisView.h"
#include <vagabond/c4x/ClusterSVD.h>
#include <vagabond/core/HasMetadata.h>
#include <vagabond/core/TorsionData.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>

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
		t->setLeft(0.21, 0.2);
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
		std::string tag = "axis_" + i_to_str(i);
		TickBoxes *tickboxes = new TickBoxes(this, this);
		tickboxes->addOption(text, tag, found);
		tickboxes->arrange(0, 0, 1.0, 0.1);
		b->addObject(tickboxes);
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
		
		auto show_axis_view = [this, i]()
		{
			TorsionAxisView *tav = new TorsionAxisView(this, _data,
			                                           _cluster, i);
			tav->show();
		};
		fb->setReturnJob(show_axis_view);
		b->addObject(fb);
	}
	
	return b;
}

void AxesMenu::supplyMainPlot()
{
	_csv = "";
	
	for (size_t i = 0; i < _data->objectCount(); i++)
	{
		HasMetadata *hm = _data->object(i);
		_csv += hm->id() + ",";
		
		for (size_t j = 0; j < _cluster->columns(); j++)
		{
			_csv += std::to_string(_cluster->value(i, j)) + ",";
		}
		
		_csv.pop_back();
		_csv += "\n";
	}
}

void AxesMenu::supplyCSV(std::string indicator)
{
	supplyMainPlot();
}

void AxesMenu::buttonPressed(std::string tag, Button *button)
{
	std::string end = Button::tagEnd(tag, "axis_");
	
	if (end.length())
	{
		TickBoxes *tb = static_cast<TickBoxes *>(button);
		int axis = atoi(end.c_str());
		_cluster->changeLastAxis(axis);
		refresh();
	}

	ListView::buttonPressed(tag, button);
	ExportsCSV::buttonPressed(tag, button);
}

