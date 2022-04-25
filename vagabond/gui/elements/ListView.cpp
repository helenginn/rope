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

#include "ListView.h"
#include "ImageButton.h"
#include <sstream>

#define LINES_PER_PAGE 8

ListView::ListView(Scene *prev) : ForwardBackward(prev)
{

}

ListView::~ListView()
{
	deleteObjects();
}

void ListView::setup()
{
	refresh();
}

size_t ListView::unitsPerPage()
{
	return LINES_PER_PAGE;
}

void ListView::refresh()
{
	loadFilesFrom(_start, LINES_PER_PAGE);
}

void ListView::loadFilesFrom(int start, int num)
{
	clearTemps();

	int count = lineCount();
	int npages = ceil((float)count / (float)LINES_PER_PAGE);
	double pos = 0.3;

	for (size_t i = start; i < start + num && i < count; i++)
	{
		Renderable *line = getLine(i);
		line->setLeft(0.2, pos);
		pos += 0.06;
		addObject(line);
		_temps.push_back(line);
	}
	
	if (npages > 1)
	{
		int mypage = _start / LINES_PER_PAGE;
		std::ostringstream ss;
		ss << "(" << mypage + 1 << " / " << npages << ")";
		Text *pageNo = new Text(ss.str());
		pageNo->setCentre(0.5, 0.8);
		addObject(pageNo);
		_temps.push_back(pageNo);

	}
	
	if (start > 0)
	{
		scrollBackButton();
	}
	
	if (count > start + num)
	{
		scrollForwardButton();
	}
}
