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

#include "Scores.h"
#include "Atom.h"

ScoreBucket::ScoreBucket(Atom *atom)
{
	chain = atom->chain();
	minRes = atom->residueId();
	maxRes = atom->residueId();
}

bool ScoreBucket::single() const
{
	return (minRes == maxRes);
}

bool ScoreBucket::fully_contains(const ScoreBucket &other) const
{
	if (other.chain != chain && chain.length() > 0 && other.chain.length() > 0)
	{
		return false;
	}
	
	if (minRes <= other.minRes && maxRes >= other.maxRes)
	{
		return true;
	}
	
	return false;
}

bool ScoreBucket::operator<(const ScoreBucket &other) const
{
	if (chain == other.chain || chain.length() == 0 || other.chain.length() == 0)
	{
		if (minRes == other.minRes)
		{
			return maxRes < other.maxRes;
		}
		else
		{
			return minRes < other.minRes;
		}
	}
	else
	{
		return chain < other.chain;
	}
}

bool ScoreBucket::operator==(const ScoreBucket &other) const
{
	return (chain == other.chain && 
	        minRes == other.minRes && 
	        maxRes == other.maxRes);
}
