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

#ifndef __vagabond__ResidueId__
#define __vagabond__ResidueId__

#include <sstream>
#include <string>

struct ResidueId
{
	int num = 1;
	std::string insert = "";
	
	ResidueId()
	{

	}
	
	ResidueId(std::string str)
	{
		char *next = nullptr;
		num = strtol(str.c_str(), &next, 10);
		insert += next;
	}
	
	ResidueId(int _num)
	{
		num = _num;
	}
	
	int as_num()
	{
		return num;
	}
	
	std::string as_string()
	{
		std::ostringstream ss;
		ss << num << insert;
		return ss.str();
	}

	const bool operator!=(const ResidueId &o) const
	{
		return (num != o.num || insert != o.insert);
	}
	
	const bool operator<(const ResidueId &o) const
	{
		if (num == o.num)
		{
			return insert < o.insert;
		}
		else
		{
			return num < o.num;
		}
	}
};

#endif
