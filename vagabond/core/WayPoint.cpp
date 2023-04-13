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

#include "WayPoint.h"
#include <vagabond/utils/polyfit.h>

std::vector<float> WayPoints::polyFit()
{
	bool changed = false;
	
	for (const WayPoint &wp : _wps)
	{
		if (wp.changed())
		{
			changed = true;
			break;
		}
	}

	if (!changed)
	{
		return _polyFit;
	}

	int n = size();
	std::vector<float> xs, ys;
	
	xs.resize(n);
	ys.resize(n);

	for (int j = 0; j < n; j++)
	{
		xs[j] = _wps[j].fraction();
		ys[j] = _wps[j].progress();
	}

	_polyFit = polyfit(xs, ys, n);

	for (WayPoint &wp : _wps)
	{
		wp.setChanged(false);
	}

	return _polyFit;
}
