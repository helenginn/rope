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
#include "RTPeptideTwist.h"
#include "WayPoint.h"
#include "Atom.h"

struct Motion
{
	WayPoints wp{};
	bool flip = false;
	float angle = 0;
	int locked = 0;
	
	struct Twist
	{
		PeptideTwist *twist = nullptr;
		bool positive = true;
		
		float contribution(float frac) const
		{
			return (positive ? 1 : -1) * twist->interpolatedTwist(frac);
		}
	};
	
	Twist twist;
	
	float workingAngle() const
	{
		if (!flip) return angle;
		
		if (angle > 0) { return angle - 360; }
		else { return angle + 360; }
	}
	
	void writeToData(DataFloat *&ptr)
	{
		float extra = twist.twist ? twist.twist->twist : 0.f;
;
		ptr[0] = (wp._amps[0] + extra * 4) * (flip ? 1 : -1);
		ptr[1] = (wp._amps[1]) * (flip ? 1 : -1);
		ptr += 2;
	}
	
	float interpolatedAngle(float frac)
	{
		float p = wp.interpolatedProgression(frac);
		float q = twist.twist ? twist.contribution(frac) : 0;
		return frac * workingAngle() + p + q;
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
	
	void incorporate(RTPeptideTwist &twist)
	{
		for (size_t j = 0; j < twist.size(); j++)
		{
			ResidueTorsion &rt = twist.rt(j);
			Parameter *p = rt.parameter();
			Atom *a = p->atom(1); // should be N or C of peptide
			Atom *b = p->atom(2); // should be N or C of peptide

			for (size_t i = 0; i < size(); i++)
			{
				ResidueTorsion &rt = this->rt(i);
				Parameter *q = rt.parameter();
				
				if (q && q->hasAtom(a) && q->hasAtom(b) && 
				    !q->isPeptideBond() && q->coversMainChain())
				{
					bool direction = q->atom(0)->elementSymbol() == "N";
					PeptideTwist &value = twist.storage(j);
					this->storage(i).twist = {&value, direction};
				}
			}
		}
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
