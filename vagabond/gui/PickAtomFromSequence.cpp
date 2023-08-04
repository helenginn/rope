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

#include "PickAtomFromSequence.h"
#include <vagabond/core/Residue.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/TextButton.h>

PickAtomFromSequence::PickAtomFromSequence(Scene *prev, IndexedSequence *sequence)
: SequenceView(prev, sequence)
{

}

void PickAtomFromSequence::addExtras(TextButton *t, Residue *r) 
{
	std::string tag = r->desc();
	t->addAltTag(tag);
	t->setInert(false);
}

void PickAtomFromSequence::handleResidue(Button *button, Residue *r)
{
	_curr = r;
	Menu *m = new Menu(this, this, "atomname");
	
	if (r->atomNames().size() == 0)
	{
		r->housekeeping();
	}
	for (const std::string &name : r->atomNames())
	{
		if (name.length() == 0 || name[0] == 'H')
		{
			continue;
		}

		m->addOption(name);
	}

	m->setup(button, 0.6);
	setModal(m);
}


void PickAtomFromSequence::buttonPressed(std::string tag, Button *button)
{
	std::string atom = Button::tagEnd(tag, "atomname_");
	
	if (atom.length())
	{
		std::pair<Residue *, std::string> atomResidue(_curr, atom);
		sendResponse("picked_atom", &atomResidue);
		back();
		return;
	}

	SequenceView::buttonPressed(tag, button);
}
