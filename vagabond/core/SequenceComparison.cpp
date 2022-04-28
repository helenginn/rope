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

#include "SequenceComparison.h"
#include "Sequence.h"

SequenceComparison::SequenceComparison(Sequence *a, Sequence *b)
{
	_left = a;
	_right = b;
	
	std::string left_str = _left->str();
	std::string right_str = _right->str();

	int muts = 0; int dels = 0;
	setup_alignment(&_leftAlign, left_str);
	setup_alignment(&_rightAlign, right_str);
	compare_sequences_and_alignments(left_str, right_str, &muts, &dels, 
	                                 _leftAlign, _rightAlign);

	tidy_alignments(_leftAlign, _rightAlign);

	std::ostringstream lss, ass, rss;
	print_alignments(_leftAlign, _rightAlign, lss, ass, rss, _indices);

	_lOutput = lss.str();
	_rOutput = rss.str();
	_aOutput = ass.str();

	delete_alignment(&_leftAlign);
	delete_alignment(&_rightAlign);
}

bool SequenceComparison::hasResidue(int row, int entry)
{
	if (row == 0 && _indices[entry].l >= 0)
	{
		return true;
	}

	if (row == 2 && _indices[entry].r >= 0)
	{
		return true;
	}
	
	return false;
}

Residue *SequenceComparison::residue(int row, int entry)
{
	if (entry < 0 || entry >= _indices.size())
	{
		return nullptr;
	}

	if (row == 0)
	{
		int idx = _indices[entry].l;
		return _left->residue(idx);
	}
	else if (row == 2)
	{
		int idx = _indices[entry].r;
		return _right->residue(idx);
	}
	else
	{
		return nullptr;
	}
}

std::string SequenceComparison::displayString(int row, int entry)
{
	if (entry < 0 || entry >= _indices.size())
	{
		return "";
	}

	std::string ret;
	if (row == 0)
	{
		ret += _lOutput[entry];
	}
	else if (row == 1)
	{
		ret += _aOutput[entry];
	}
	else if (row == 2)
	{
		ret += _rOutput[entry];
	}
	
	return ret;
}

float SequenceComparison::match()
{
	float count = 0;
	for (size_t i = 0; i < _aOutput.size(); i++)
	{
		if (_aOutput[i] == '.')
		{
			count++;
		}
	}
	
	count /= (float)_aOutput.size();
	return count;
}
