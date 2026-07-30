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

#include "ResidueRange.h"
#include <vagabond/utils/FileReader.h>
#include <cctype>
#include <cstdlib>

bool parseResidueRanges(const std::string &plan,
                        std::vector<ResidueRangeToken> &tokens,
                        std::string &error)
{
	std::string currentChain;
	std::vector<std::string> bits = split(plan, ',');

	for (std::string bit : bits)
	{
		trim(bit);

		size_t i = 0;
		while (i < bit.length() && std::isalpha((unsigned char)bit[i]))
		{
			i++;
		}

		if (i > 0)
		{
			currentChain = bit.substr(0, i);
			bit = bit.substr(i);
		}

		std::vector<std::string> sides = split(bit, '-');

		if (sides.size() > 2)
		{
			error = "Too many '-' in \"" + bit + "\".";
			return false;
		}

		for (std::string &side : sides)
		{
			trim(side);
		}

		int begin = atoi(sides[0].c_str());
		int end = begin;

		if (sides.size() == 2)
		{
			end = atoi(sides[1].c_str());

			if (end < begin)
			{
				error = "Range end before start in \"" + bit + "\".";
				return false;
			}
		}

		tokens.push_back({currentChain, begin, end});
	}

	return true;
}
