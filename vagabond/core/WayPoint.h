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

class WayPoint
{
public:
	WayPoint()
	{

	}
	
	void setFraction(float f)
	{
		_fraction = f;
		_changed = true;
	}

	void setProgress(float p)
	{
		_progress = p;
		_changed = true;
	}
	
	float &fraction_var() 
	{
		_changed = true;
		return _fraction;
	}

	float &progress_var()
	{
		_changed = true;
		return _progress;
	}

	const float &fraction() const
	{
		return _fraction;
	}

	const float &progress() const
	{
		return _progress;
	}

	WayPoint(float f, float p)
	{
		_fraction = f; _progress = p; _changed = true;
	}

	static WayPoint startPoint()
	{
		WayPoint wp;
		wp._fraction = 0;
		wp._progress = 0;
		return wp;
	}

	static WayPoint midPoint()
	{
		WayPoint wp;
		wp._fraction = 0.5;
		wp._progress = 0.5;
		return wp;
	}

	static WayPoint endPoint()
	{
		WayPoint wp;
		wp._fraction = 1;
		wp._progress = 1;
		return wp;
	}
	
	friend std::ostream &operator<<(std::ostream &ss, const WayPoint &wp)
	{
		ss << "(" << wp._fraction << ", " << wp._progress << "), ";
		ss << (wp._changed ? "*" : "");
		return ss;
	}
	
	const bool &changed() const
	{
		return _changed;
	}
	
	void setChanged(bool changed)
	{
		_changed = changed;
	}

	std::vector<float> polyFit();

	friend void to_json(json &j, const WayPoint &value);
	friend void from_json(const json &j, WayPoint &value);
private:
	float _fraction = 0.5; /* fraction of route progression between 0 and 1 */
	float _progress = 0.5; /* proportion of progress to goal at WayPoint */

	bool _changed = true;
};
	
struct WayPoints
{
	std::vector<WayPoint> _wps;

	std::vector<float> polyFit();
	
	const WayPoint &at(int i) const
	{
		return _wps.at(i);
	}
	
	WayPoint &operator[](int i)
	{
		return _wps[i];
	}
	
	void push_back(WayPoint wp)
	{
		_wps.push_back(wp);
	}
	
	const size_t size() const
	{
		return _wps.size();
	}
	
	std::vector<float> _polyFit;
};

/* waypoint */
inline void to_json(json &j, const WayPoint &value)
{
	j["p"] = value._progress;
	j["f"] = value._fraction;
}

/* waypoint */
inline void from_json(const json &j, WayPoint &value)
{
    value._progress = j.at("p");
    value._fraction = j.at("f");
}

/* waypoints */
inline void to_json(json &j, const WayPoints &value)
{
	j["wps"] = value._wps;
}

/* waypoint */
inline void from_json(const json &j, WayPoints &value)
{
    value._wps = j.at("wps");
}

#endif
