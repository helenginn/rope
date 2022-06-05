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

#ifndef __vagabond__SequenceComparison__
#define __vagabond__SequenceComparison__

#include <vagabond/utils/seqalign.h>

#include "IndexedSequence.h"

class Sequence;
class Residue;

class SequenceComparison : public IndexedSequence
{
public:
	SequenceComparison(Sequence *a, Sequence *b);

	virtual size_t rowCount()
	{
		return 3;
	}

	virtual size_t entryCount()
	{
		return _indices.size();
	}

	virtual Residue *residue(int row, int entry);
	virtual bool hasResidue(int row, int entry);
	virtual std::string displayString(int row, int entry);
	
	void calculateMutations();
	
	const std::vector<std::string> &mutations()
	{
		return _muts;
	}
	
	Sequence *right()
	{
		return _right;
	}

	const Alignment &leftAlign() const
	{
		return _leftAlign;
	}

	const Alignment &rightAlign() const
	{
		return _rightAlign;
	}
	
	float match();
private:
	Sequence *_left = nullptr;
	Sequence *_right = nullptr;

	Alignment _leftAlign, _rightAlign;

	IndexPairs _indices;
	std::string _lOutput;
	std::string _rOutput;
	std::string _aOutput;
	std::vector<std::string> _muts;
};

#endif
