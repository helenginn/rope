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
	
	for (size_t i = 0; i < _leftAlign.seq.size(); i++)
	{
		std::cout << (int)_leftAlign.map[i] << " ";
	}
	std::cout << std::endl;

	delete_alignment(&_leftAlign);
	delete_alignment(&_rightAlign);
	
	std::cout << _lOutput << std::endl;
	std::cout << _aOutput << std::endl;
	std::cout << _rOutput << std::endl;
}

bool SequenceComparison::hasResidue(int row, int entry)
{
	if (row == 2 && _rOutput[row] != ' ')
	{
		return true;
	}

	if (row == 0 && _lOutput[row] != ' ')
	{
		return true;
	}
	
	return false;
}

Residue *SequenceComparison::residue(int row, int entry)
{
	if (entry < 0 || _indices.size() >= entry)
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
	if (entry < 0 || _indices.size() >= entry)
	{
		return "";
	}

	std::string ret;
	if (row == 0)
	{
		ret += _lOutput[row];
	}
	else if (row == 1)
	{
		ret += _aOutput[row];
	}
	else if (row == 2)
	{
		ret += _rOutput[row];
	}
	
	return ret;
}
