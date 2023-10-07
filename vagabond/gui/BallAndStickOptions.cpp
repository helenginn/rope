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

#include "BallAndStickOptions.h"

#include <vagabond/utils/FileReader.h>

#include <vagabond/core/Entity.h>

#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/TextButton.h>

BallAndStickOptions::BallAndStickOptions(Scene *prev, Entity *ent) : ListView(prev)
{
	_entity = ent;
	_preferences = &(_entity->visualPreferences());
	Sequence *seq = _entity->sequence();
}

void BallAndStickOptions::setup()
{
	addTitle("Ball and stick options");

	ListView::setup();
}

size_t BallAndStickOptions::lineCount()
{
	return _preferences->displayBallSticks().size() + 1;

}

Renderable *BallAndStickOptions::getLine(int i)
{
	if (i == 0)
	{
		TextButton *add = new TextButton("(+) add residue", this);
		add->setReturnTag("add");
		return add;
	}

	i--;

	Sequence *seq = _entity->sequence();
	ResidueId resId = _preferences->displayBallSticks()[i];
	Residue *find_res = seq->residue(resId);
	std::string id = find_res->desc();
	
	Box *b = new Box();
	Text *t = new Text(id);
	t->setLeft(0., 0.);
	b->addObject(t);

	ImageButton *ib = new ImageButton("assets/images/cross.png", this);
	ib->resize(0.06);
	ib->setRight(0.6, 0.0);
	ib->setReturnTag("__del_" + std::to_string(i));
	b->addObject(ib);

	return b;
}

bool BallAndStickOptions::addResidue(int res)
{
	std::cout << "Attempting add " << res << std::endl;
	ResidueId resId = ResidueId(res);
	Sequence *seq = _entity->sequence();

	Residue *find_res = seq->residueLike(resId);
	
	if (find_res == nullptr)
	{
		return false;
	}
	
	const ResidueId &realId = find_res->id();
	_preferences->addBallStickResidue(realId);
	return true;
}

void BallAndStickOptions::addResidueRange(std::string range)
{
	bool failed = 0;
	int count = 0;
	int not_found = 0;
	std::vector<std::string> bits = split(range, ',');

	for (std::string &bit : bits)
	{
		std::vector<std::string> sides = split(bit, '-');
		
		if (sides.size() > 2)
		{
			failed = true;
			break;
		}
		
		for (std::string &side : sides)
		{
			trim(side);
		}
		
		int begin = atoi(sides[0].c_str());
		if (sides.size() == 1)
		{
			bool found = addResidue(begin);
			found ? count++ : not_found++;
			continue;
		}
		
		int end = atoi(sides[1].c_str());
		
		if (end < begin)
		{
			failed = true;
			break;
		}

		for (size_t res = begin; res <= end; res++)
		{
			bool found = addResidue(res);
			found ? count++ : not_found++;
		}
	}
	
	std::string str = "Added " + std::to_string(count) + ", missed "
	+ std::to_string(not_found) + " residues.\n";
	if (failed)
	{
		str += "Aborted early due to syntax errors.";
	}
	BadChoice *bc = new BadChoice(this, str);
	setModal(bc);
	
	refresh();
}

void BallAndStickOptions::buttonPressed(std::string tag, Button *button)
{
	std::string end = Button::tagEnd(tag, "__del_");
	if (tag == "add")
	{
		AskForText *aft = new AskForText(this, "Residue range",
		                                 "add_residues", this);
		setModal(aft);
	}
	else if (tag == "add_residues")
	{
		std::string range = static_cast<TextEntry *>(button)->scratch();
		addResidueRange(range);
	}
	else if (end.length() > 0)
	{
		int num = atoi(end.c_str());
		_preferences->removeBallStickResidue(num);
		refresh();
	}

	ListView::buttonPressed(tag, button);
}
