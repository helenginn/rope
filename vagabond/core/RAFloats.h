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

#ifndef __vagabond__RAFloats__
#define __vagabond__RAFloats__

#include "RAVector.h"
#include <vagabond/c4x/Angular.h>
#include <sstream>

class RAFloats : public RAVector<DataFloat>
{
public:
	static RAFloats floats_from(const std::vector<Atom3DPosition> &rts,
	                            const std::vector<DataFloat> &floats = {})
	{
		RAFloats res;
		res.vector_from(rts, floats);
		return res;
	}
	
	friend std::ostream &operator<<(std::ostream &ss, 
	                                const RAFloats &floats)
	{
		for (int i = 0; i < floats.size() && i < 20; i++)
		{
			ss << floats.storage(i) << ", ";
		}
		ss << "...";
		return ss;
	}
	
	void order_by_magnitude()
	{
		std::sort(_pairs.begin(), _pairs.end(), 
		          [](const RAFloats::HeaderValue &a, 
		             const RAFloats::HeaderValue &b) -> bool
		{
			return (a.storage * a.storage > b.storage * b.storage);

		});
	}
	
	const DataFloat &value(int i) const
	{
		return this->storage(i);
	}
private:

};

#endif
