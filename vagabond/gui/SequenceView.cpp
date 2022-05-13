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

#include "SequenceLine.h"
#include "SequenceView.h"
#include <vagabond/core/Sequence.h>
#include <vagabond/core/Residue.h>
#include "IndexedSequence.h"
#include <iostream>
#include <vagabond/utils/FileReader.h>

#include <vagabond/gui/elements/TextButton.h>

#define ROWS_PER_PAGE 10

SequenceView::SequenceView(Scene *prev, IndexedSequence *sequence) 
: ForwardBackward(prev)
{
	_sequence = sequence;
	_start = 0;
}

void SequenceView::setup()
{
	addTitle("Sequence view");

	getLineLimits();
	loadLines();
}

size_t SequenceView::unitsPerPage()
{
	return _residuesPerPage;
}

size_t SequenceView::linesPerPage()
{
	int rows = _sequence->rowCount() + 1;
	rows = ROWS_PER_PAGE / rows;
	
	if (rows == 0)
	{
		rows = 1;
	}
	
	return rows;
}

void SequenceView::refresh()
{
	loadLines();
}

void SequenceView::getLineLimits()
{
	_maxLines = linesPerPage();
	int resCount = RESIDUES_PER_ROW;
	_residuesPerPage = resCount * _maxLines;
}

void SequenceView::loadLines()
{
	clearTemps();

	double pos = 0.3;
	int rows = _sequence->rowCount() + 1;
	int end = _start + _residuesPerPage;
	if (end > _sequence->entryCount()) end = _sequence->entryCount();

	for (size_t i = _start; i < end; i += RESIDUES_PER_ROW)
	{
		SequenceLine *line = new SequenceLine(this, _sequence, i);
		line->setup();
		line->setLeft(0.2, pos);
		pos += 0.06 * rows;
		addObject(line);
		_temps.push_back(line);
	}
	
	if (_start + _residuesPerPage < _sequence->entryCount())
	{
		scrollForwardButton();
	}
	if (_start > 0)
	{
		scrollBackButton();
	}
}

void SequenceView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "residue")
	{
		Residue *r = static_cast<Residue *>(button->returnObject());
		handleResidue(button, r);
	}

	ForwardBackward::buttonPressed(tag, button);
}

TextButton *SequenceView::button(Sequence *seq, ButtonResponder *caller)
{
	int numres = seq->size();
	std::string seqstr = i_to_str(numres) + " res.";

	TextButton *t = new TextButton(seqstr, caller);
	return t;
}

void SequenceView::addExtras(TextButton *t, Residue *r)
{
	std::string tag = r->desc();
	if (r->torsionCount() > 0)
	{
		tag += " (" + i_to_str(r->torsionCount()) + " torsions)";

	}
	t->addAltTag(tag);
}
