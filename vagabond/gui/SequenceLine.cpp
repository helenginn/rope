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

#include "Residue.h"
#include "SequenceLine.h"
#include "IndexedSequence.h"
#include <vagabond/gui/elements/Text.h>
#include <vagabond/utils/FileReader.h>
#include <iostream>

SequenceLine::SequenceLine(SequenceView *me, IndexedSequence *sequence, int start)
{
	_view = me;
	_sequence = sequence;
	_start = start;
	_end = _start + RESIDUES_PER_ROW;
	if (_end > _sequence->entryCount())
	{
		_end = _sequence->entryCount();
	}
}

SequenceLine::~SequenceLine()
{

}

void SequenceLine::addRightIndicator(int entry, float x)
{
	if (!_sequence->hasResidue(0, entry))
	{
		return;
	}
	Residue *r = _sequence->residue(0, entry);
	int num = r->as_num();
	std::string str = " - " + i_to_str(num);
	Text *t = new Text(str);
	t->setLeft(x, 0);
	addObject(t);
}

void SequenceLine::addLeftIndicator(int entry)
{
	if (!_sequence->hasResidue(0, entry))
	{
		return;
	}
	Residue *r = _sequence->residue(0, entry);
	int num = r->as_num();
	std::string str = i_to_str(num) + " -  ";
	Text *t = new Text(str);
	t->setRight(0, 0);
	addObject(t);
}

void SequenceLine::setup()
{
	float x = 0;
	float y = 0;
	for (size_t j = 0; j < _sequence->rowCount(); j++)
	{
		addLeftIndicator(_start);

		for (size_t i = _start; i < _end; i++)
		{
			std::string str = _sequence->displayString(j, i);
			
			Text *t = new Text(str);
			if (_sequence->hasResidue(j, i))
			{
				Residue *r = _sequence->residue(j, i);
				std::string tag = r->desc();
				if (r->torsionCount() > 0)
				{
					tag += " (" + i_to_str(r->torsionCount()) + " torsions)";

				}
				t->addAltTag(tag);
			}

			t->setCentre(x, y);
			addObject(t);
			
			x += 0.02;
		}

		addRightIndicator(_end - 1, x);

		y += 0.06;
		x = 0;
	}
}
