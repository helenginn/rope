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

#ifndef __vagabond__RAMovement__
#define __vagabond__RAMovement__

#include "RAVector.h"
#include <vagabond/c4x/Posular.h>
#include <sstream>

class RAMovement : public RAVector<Posular>
{
public:
	static RAMovement movements_from(const std::vector<Atom3DPosition> &ras,
	                                 const std::vector<Posular> &ps)
	{
		RAMovement res;
		res.vector_from(ras, ps);
		return res;
	}
	
	friend std::ostream &operator<<(std::ostream &ss, 
	                                const RAMovement &movements)
	{
		for (int i = 0; i < movements.size() && i < 20; i++)
		{
			ss << movements.storage(i) << ", ";
		}
		ss << "...";
		return ss;
	}
	

private:
	
};

#endif
