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

#include "DistanceMaker.h"
#include <vagabond/core/Residue.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Entity.h>
#include <vagabond/gui/MetadataView.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <iostream>

DistanceMaker::DistanceMaker(Scene *prev, IndexedSequence *sequence)
: SequenceView(prev, sequence)
{

}

void DistanceMaker::addExtras(TextButton *t, Residue *r) 
{
	t->setInert(false);
}

void DistanceMaker::handleResidue(Button *button, Residue *r)
{
	_curr = r;
	glm::vec2 c = button->xy();
	Menu *m = new Menu(this, "atomname");
	
	for (const std::string &name : r->atomNames())
	{
		if (name.length() == 0 || name[0] == 'H')
		{
			continue;
		}

		m->addOption(name);
	}

	m->setup(c.x, c.y);
	setModal(m);
}

void DistanceMaker::handleAtomName(std::string name)
{
	_candidate = name;
	std::string mesg;

	if (_stage == Nothing)
	{
		mesg = "Measure distance from atom " + _curr->desc() + name + "?";
	}
	else if (_stage == ChosenFirst)
	{
		mesg = "Measure distance to atom " + _curr->desc() + name + "?";
	}
	
	std::cout << mesg << std::endl;

	AskYesNo *ayn = new AskYesNo(this, mesg, "confirm_atom", this);
	setModal(ayn);

}

void DistanceMaker::confirmAtom()
{
	if (_stage == Nothing)
	{
		_first = _candidate;
		_aRes = _curr;
		_stage = ChosenFirst;
	}
	else if (_stage == ChosenFirst)
	{
		_second = _candidate;
		_bRes = _curr;
		calculateDistance();
	}
}

void DistanceMaker::calculateDistance()
{
	Metadata *md = _entity->distanceBetweenAtoms(_aRes, _first, _bRes, _second);
	
	MetadataView *mv = new MetadataView(this, md);
	mv->show();
	_stage = Nothing;
}

void DistanceMaker::buttonPressed(std::string tag, Button *button)
{
	std::cout << tag << std::endl;
	std::string atom = Button::tagEnd(tag, "atomname_");
	
	if (atom.length())
	{
		handleAtomName(atom);
		return;
	}
	
	std::string yes = Button::tagEnd(tag, "yes_");
	std::cout << "Yes: " << yes << std::endl;
	
	if (yes == "confirm_atom")
	{
		confirmAtom();
		return;
	}

	SequenceView::buttonPressed(tag, button);
}
