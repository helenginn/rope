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
#include "TextEntry.h"
#include "TextButton.h"
#include "AskForText.h"
#include <sstream>

#define LINES_PER_PAGE 8

ListView::ListView(Scene *prev) : ForwardBackward(prev)
{

}

ListView::~ListView()
{
	
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
	loadFilesFrom(_start, unitsPerPage());
}

int ListView::nPages()
{
	int count = lineCount();
	int npages = ceil((float)count / (float)unitsPerPage());
	return npages;
}

void ListView::loadFilesFrom(int start, int num)
{
	deleteTemps();

	int count = lineCount();
	int npages = nPages();
	double pos = 0.3;

	for (size_t i = start; i < start + num && i < count; i++)
	{
		Renderable *line = getLine(i);
		line->setLeft(leftMargin(), pos);
		float ratio = 0.06 * LINES_PER_PAGE / (float)unitsPerPage();
		pos += ratio;
		addTempObject(line);
	}
	
	if (npages > 1)
	{
		int mypage = _start / unitsPerPage();
		std::ostringstream ss;
		ss << "(" << mypage + 1 << " / " << npages << ")";
		TextButton *pageNo = new TextButton(ss.str(), this);
		pageNo->setCentre(0.5, 0.8);
		pageNo->setReturnJob([this]()
		{
			AskForText *aft = new AskForText(this, "Jump to page:", 
			                                 "change_page", this,
			                                 TextEntry::Numeric);
			setModal(aft);
		});

		addTempObject(pageNo);

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

void ListView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "change_page")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		int page = atoi(te->scratch().c_str());
		int npages = nPages();
		if (page >= 1 && page <= npages)
		{
			page--;
			_start = page * unitsPerPage();
			refresh();
		}
	}

	ForwardBackward::buttonPressed(tag, button);
}
