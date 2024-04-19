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

#ifndef __vagabond__WayPoint__
#define __vagabond__WayPoint__

#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
using nlohmann::json;
	
struct WayPoints
{
	WayPoints();
	WayPoints(int order);

	float interpolatedProgression(float frac);
	
	std::vector<float> _grads = {0, 0};
};

/* waypoints */
inline void to_json(json &j, const WayPoints &value)
{
	j["params"] = value._grads;
}

/* waypoint */
inline void from_json(const json &j, WayPoints &value)
{
	if (j.count("params"))
	{
		std::vector<float> params = j.at("params");
		value._grads = params;
	}
}

#endif
