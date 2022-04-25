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

#ifndef __vagabond__SequenceLine__
#define __vagabond__SequenceLine__

#define RESIDUES_PER_ROW 30

#include <vagabond/gui/elements/Box.h>

class IndexedSequence;
class SequenceView;
class TextButton;
class Sequence;

class SequenceLine : public Box
{
public:
	SequenceLine(SequenceView *me, IndexedSequence *seq, int start);
	
	~SequenceLine();

	void setup();
private:
	void addLeftIndicator(int entry);
	void addRightIndicator(int entry, float x);

	SequenceView *_view;
	IndexedSequence *_sequence;
	int _start;
	int _end;

};

#endif
