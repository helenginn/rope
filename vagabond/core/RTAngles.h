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

#ifndef __vagabond__RTAngles__
#define __vagabond__RTAngles__

#include "RTVector.h"
#include <vagabond/c4x/Angular.h>
#include <sstream>

class RTAngles : public RTVector<Angular>
{
public:
	static RTAngles angles_from(const std::vector<ResidueTorsion> &rts,
	                            const std::vector<Angular> &angles = {})
	{
		RTAngles res;
		res.vector_from(rts, angles);
		return res;
	}
	
	friend std::ostream &operator<<(std::ostream &ss, 
	                                const RTAngles &angles)
	{
		for (int i = 0; i < angles.size() && i < 20; i++)
		{
			ss << angles.storage(i) << ", ";
		}
		ss << "...";
		return ss;
	}
	
	void match_degrees_to(RTAngles &other)
	{
		for (int i = 0; i < size(); i++)
		{
			const float &ref = other.storage(i);

			while (storage(i) < ref - 180.f)
			{
				storage(i) += 360.f;
			}

			while (storage(i) >= ref + 180.f)
			{
				storage(i) -= 360.f;
			}
		}
	}
	
	void order_by_magnitude()
	{
		std::sort(_pairs.begin(), _pairs.end(), 
		          [](const RTAngles::HeaderValue &a, 
		             const RTAngles::HeaderValue &b) -> bool
		{
			return (a.storage * a.storage > b.storage * b.storage);

		});


	}
	
	const Angular &angle(int i) const
	{
		return this->storage(i);
	}
private:

};

#endif
