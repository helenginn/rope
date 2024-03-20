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

#ifndef __vagabond__RTPeptideTwist__
#define __vagabond__RTPeptideTwist__

#include "RTVector.h"

class PeptideTwist
{
public:
	float twist = 0.f;
	
	float interpolatedTwist(float frac) const
	{
		return (twist*4) * frac - (twist*4) * frac * frac;
	}

private:

};

inline void to_json(json &j, const PeptideTwist &id)
{
	j["twist"] = id.twist;
}

inline void from_json(const json &j, PeptideTwist &id)
{
	j.at("twist").get_to(id.twist);
}

class RTPeptideTwist : public RTVector<PeptideTwist>
{
public:
	static RTPeptideTwist empty_twists(const std::vector<ResidueTorsion> &rts)
	{
		RTPeptideTwist res;
		
		for (ResidueTorsion t : rts)
		{
			Parameter *p = t.parameter();
			if (p && p->isPeptideBond())
			{
				res.addResidueTorsion(t, {});
			}

		}

		return res;
	}

	static RTPeptideTwist twists_for(const std::vector<ResidueTorsion> &rts,
	                                 const std::vector<PeptideTwist> &twists)
	{
		RTPeptideTwist res;
		res.vector_from(rts, twists);
		return res;
	}

	void twists_from(const std::vector<float> &dest)
	{
		assert(dest.size() == _pairs.size());
		
		for (int i = 0; i < dest.size(); i++)
		{
			_pairs[i].storage.twist = dest[i];
		}
	}

	friend void to_json(json &j, const RTPeptideTwist &value);
	friend void from_json(const json &j, RTPeptideTwist &value);
private:

};

inline void to_json(json &j, const RTVector<PeptideTwist>::RTValue &id)
{
	j["rt"] = id.header;
	j["storage"] = id.storage;

}

inline void from_json(const json &j, RTVector<PeptideTwist>::RTValue &id)
{
	id.header = j.at("rt");
	id.storage = j.at("storage");
}

inline void to_json(json &j, const RTPeptideTwist &id)
{
	j = id._pairs;
}

inline void from_json(const json &j, RTPeptideTwist &id)
{
	std::vector<RTPeptideTwist::RTValue> values = j;
	id._pairs = values;
}


#endif
