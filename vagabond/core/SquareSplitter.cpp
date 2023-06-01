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

#include <algorithm>
#include <iostream>
#include <float.h>
#include "SquareSplitter.h"
#include <vagabond/utils/svd/PCA.h>

SquareSplitter::SquareSplitter(PCA::Matrix &mat) : _mat(mat)
{
	split();
}

bool SquareSplitter::valid(int i, int j, const std::vector<int> &after)
{
	for (int k = -1; k < (int)after.size(); k++)
	{
		int start = (k < 0 ? 0 : after[k]);
		int end = (k == after.size() - 1 ? _mat.rows : after[k + 1]);
		
		if (start <= i && i < end && start <= j && j < end) 
		{
			return true;
		}
	}
	
	return false;
}

float SquareSplitter::score_to_split(int spl)
{
	std::vector<int> copy = _after;
	copy.push_back(spl);
	std::sort(copy.begin(), copy.end());
	float score = 0;

	for (size_t i = 0; i < _mat.rows; i++)
	{
		for (size_t j = 0; j < _mat.cols; j++)
		{
			if (valid(i, j, copy))
			{
				score += _mat[i][j] * _mat[i][j];
			}
		}
	}

	score = sqrt(score);
	return score;
}

void SquareSplitter::next_split()
{
	int best = 0;
	float lowest = FLT_MAX;

	for (size_t i = 0; i < _mat.rows - 1; i++)
	{
		float score = score_to_split(i);
		
		if (score < lowest)
		{
			lowest = score;
			best = i;
		}
	}
	
	_after.push_back(best);
}

void SquareSplitter::split()
{
	for (size_t i = 0; i < 5; i++)
	{
		next_split();
	}

}
