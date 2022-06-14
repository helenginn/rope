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
	if (_indices.size() <= entry)
	{
		return false;
	}

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
	float good = 0;
	float all = 0;
	for (size_t i = 0; i < _aOutput.size(); i++)
	{
		if (_aOutput[i] == '.')
		{
			good++;
		}
		
		if (_rOutput[i] != ' ')
		{
			all++;
		}
	}
	
	float match = good / all;
	return match;
}

void SequenceComparison::calculateMutations()
{
	for (size_t i = 0; i < _aOutput.size(); i++)
	{
		if (_aOutput[i] == '.')
		{
			continue;
		}

		if (_aOutput[i] == '+' || _aOutput[i] == '-')
		{
			continue;
		}
		
		if (_aOutput[i] != ' ')
		{
			continue;
		}

		/* now dealing with ' ' only */
		/* firstly, find the bounds of the mutated range. */
		size_t j = i;
		for (j = i; (j < _aOutput.size() && j < _lOutput.size()
		             && j < _rOutput.size()); j++)
		{
			if (_aOutput[j] != ' ')
			{
				break;
			}
		}
		
		/* runs from i -> j but we must check for Xs. Xs come from disordered
		 * regions that we don't want to report on */
		
		bool end = false;
		for (size_t n = i; n < j; n++)
		{
			if (n >= _lOutput.size() || n >= _rOutput.size())
			{
				end = true;
			}

			if (_lOutput[n] == 'X' || _rOutput[n] == 'X')
			{
				/* truncate mutated region */
				j = n;
			}
		}
		
		/* give up if this truncates mutated region to nothing */
		if (j == i || end)
		{
			continue;
		}
		
		if (j >= _rOutput.size() || j >= _lOutput.size() ||
		    i >= _rOutput.size() || i >= _lOutput.size())
		{
			continue;
		}
		
		std::string left, right;
		for (size_t n = i; n < j; n++)
		{
			left += _lOutput[n];
			right += _rOutput[n];
		}
		
		if (left.length() == 0 || right.length() == 0)
		{
			continue;
		}

		Residue *first = residue(0, i);
		
		if (first)
		{
			std::string num = first->id().as_string();

			std::string mut = left + num + right;
			std::cout << mut << " ";
			_muts.push_back(mut);

			i = j;
		}
	}
	
	std::cout << " - total " << _muts.size() << " mutations." << std::endl;
}
