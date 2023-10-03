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

#ifndef __vagabond__AcquireCoord__
#define __vagabond__AcquireCoord__

#include <vector>
#include <functional>

namespace Coord
{
	typedef std::function<float(int idx)> Get;

	template <typename Type> 
	using Interpolate = std::function<Type(const Coord::Get &)>;

	inline Get convertedGet(const Get &get, 
	                        std::function<int(const int &idx)> &convert)
	{
		Get converted_get = [get, convert](int idx) -> float
		{
			if (idx < 0)
			{
				return 0.f;
			}

			return get(convert(idx));
		};

		return converted_get;

	}

	template <typename Type>
	Get fromVector(const std::vector<Type> &all)
	{
		Get get = [all](int idx)
		{
			return all[idx];
		};

		return get;
	}

}

#endif
