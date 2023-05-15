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

#include "Summary.h"
#include "PathTask.h"
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/Image.h>
#include <sstream>

Summary::Summary(PathTask *top) : SimplePolygon()
{
	_top = top;
	setName("Summary");
	
	setup();
}

Image *Summary::prepareImage(std::string file, float top)
{
	Image *image = new Image(file);
	image->resize(0.05);
	image->setLeft(0.4, top);
	image->setDisabled(true);
	addObject(image);

	return image;
}

Summary::TickLine Summary::makeLine(float top, std::string words)
{
	Text *text = new Text(words);
	text->setLeft(0.45, top);
	addObject(text);

	Image *tools = prepareImage("assets/images/tools.png", top);
	Image *tick = prepareImage("assets/images/tick.png", top);
	Image *cross = prepareImage("assets/images/cross.png", top);
	
	TickLine tl{text, tools, tick, cross};
	
	return tl;
}

void Summary::setup()
{
	_pathFinding = makeLine(0.3, "Finding sensible paths");
	_pathFinding.running->setDisabled(false);

	_warping = makeLine(0.36, "Warping linear space");
}

void Summary::finishPathFinding()
{
	_pathFinding.disableAll();
	_pathFinding.tick->setDisabled(false);

	_warping.disableAll();
	_warping.running->setDisabled(false);
}

void Summary::TickLine::disableAll()
{
	running->setDisabled(true);
	tick->setDisabled(true);
	cross->setDisabled(true);
}
