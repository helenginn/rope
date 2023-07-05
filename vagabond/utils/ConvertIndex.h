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

#ifndef __vagabond__ConvertIndex__
#define __vagabond__ConvertIndex__

#include <iostream>

namespace Index
{
	typedef std::function<int(const int &idx)> Convert;

	inline Convert identity()
	{
		Convert conv = [](int idx)
		{
			return idx;
		};

		return conv;
	}

	inline Convert from_list(const std::vector<int> &mapped)
	{
		std::map<int, int> lookup;
		for (size_t i = 0; i < mapped.size(); i++)
		{
			lookup[mapped[i]] = i;
		}

		Convert conv = [lookup](int idx)
		{
			if (lookup.count(idx) == 0)
			{
				return -1;
			}
			
			return lookup.at(idx);
		};

		return conv;
	}
}

#endif
