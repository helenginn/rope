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

#ifndef __vagabond__RTMotion__
#define __vagabond__RTMotion__

#include <vagabond/c4x/DataFloat.h>
#include "RTVector.h"
#include "WayPoint.h"
#include "Atom.h"

struct Motion
{
	WayPoints wp{};
	bool flip = false;
	float angle = 0;
	int locked = 0;
	
	float workingAngle() const
	{
		if (!flip) return angle;
		
		if (angle > 0) { return angle - 360; }
		else { return angle + 360; }
	}
	
	float fractionalWorkingAngle(const float &frac)
	{
		return frac * workingAngle();
	}
	
	void writeToData(DataFloat *&ptr)
	{
		ptr[0] = (wp._amps[0]) * (flip ? 1 : -1);
		ptr[1] = (wp._amps[1]) * (flip ? 1 : -1);
		ptr += 2;
	}
	
	float interpolatedAngle(float frac)
	{
		float p = wp.interpolatedProgression(frac);
		float q = fractionalWorkingAngle(frac);
		return q + p;
	}
	
	float hiddenHingeAngle(const float &frac)
	{
		float p = interpolatedAngle(frac);
		float w = workingAngle();
		if (p * w < 0)
		{
			return fabs(p);
		}

		float diff = fabs(p) - fabs(workingAngle());

		float result = std::max(0.f, diff);

		return result;
	}
};

inline void to_json(json &j, const Motion &id)
{
	j["wp"] = id.wp;
	j["flip"] = id.flip;
	j["angle"] = id.angle;
}

inline void from_json(const json &j, Motion &id)
{
	j.at("wp").get_to(id.wp);
	j.at("flip").get_to(id.flip);
	j.at("angle").get_to(id.angle);
}

class RTMotion : public RTVector<Motion>
{
public:
	static RTMotion motions_from(const std::vector<ResidueTorsion> &rts,
	                             const std::vector<Motion> &motions)
	{
		RTMotion res;
		res.vector_from(rts, motions);
		return res;
	}
	
	void flips_from(const std::vector<bool> &flips)
	{
		assert(flips.size() == _pairs.size());
		
		for (int i = 0; i < flips.size(); i++)
		{
			_pairs[i].storage.flip = flips[i];
		}
	}

	void waypoints_from(const std::map<int, WayPoints> &wps)
	{
		for (auto it = wps.begin(); it != wps.end(); it++)
		{
			_pairs[it->first].storage.wp = it->second;
		}
	}

	void motion_angles_from(const std::vector<float> &dest)
	{
		assert(dest.size() == _pairs.size());
		
		for (int i = 0; i < dest.size(); i++)
		{
			_pairs[i].storage.angle = dest[i];
		}
	}

	friend void to_json(json &j, const RTMotion &value);
	friend void from_json(const json &j, RTMotion &value);
private:

};

inline void to_json(json &j, const RTVector<Motion>::RTValue &id)
{
	j["rt"] = id.header;
	j["storage"] = id.storage;

}

inline void from_json(const json &j, RTVector<Motion>::RTValue &id)
{
	id.header = j.at("rt");
	id.storage = j.at("storage");
}

inline void to_json(json &j, const RTMotion &id)
{
	j = id._pairs;

}

inline void from_json(const json &j, RTMotion &id)
{
	std::vector<RTMotion::RTValue> values = j;
	id._pairs = values;
}


#endif
