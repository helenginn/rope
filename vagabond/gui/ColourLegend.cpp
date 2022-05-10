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

#include "ColourLegend.h"
#include <vagabond/gui/elements/Text.h>
#include <vagabond/utils/FileReader.h>

ColourLegend::ColourLegend(Scheme scheme) : ColourScheme(scheme)
{
	resize(0.25);
}

ColourLegend::~ColourLegend()
{
	deleteObjects();
}

void ColourLegend::setTitle(std::string title)
{
	Text *text = new Text(title);
	text->resize(0.6);
	text->setCentre(0.5, 0.05);
	addObject(text);
}

void ColourLegend::setLimits(float min, float max)
{
	std::string minstr = f_to_str(min, 2);
	std::string maxstr = f_to_str(max, 2);

	{
		Text *text = new Text(minstr);
		text->resize(0.6);
		text->setCentre(0.375, 0.15);
		addObject(text);
	}

	{
		Text *text = new Text(maxstr);
		text->resize(0.6);
		text->setCentre(0.625, 0.15);
		addObject(text);
	}
}
