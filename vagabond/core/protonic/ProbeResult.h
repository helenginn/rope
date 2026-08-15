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

#ifndef __vagabond__ProbeResult__
#define __vagabond__ProbeResult__

#include <vagabond/core/protonic/hnet.h>
#include <vagabond/core/protonic/Probe.h>

class Probe;

typedef std::function<float()> GetScore;

struct ProbeTypePair : public std::pair<Probe *, hnet::Types>
{
	ProbeTypePair(const std::pair<Probe *, hnet::Types> &orig = {})
	{
		this->first = orig.first;
		this->second = orig.second;
	}

	bool operator<(const ProbeTypePair &other) const
	{
		if (!this->first || !other.first)
		{
			return this->first < other.first;
		}
		else if (this->second == other.second)
		{
			if (this->first->atomConf().ptr && other.first->atomConf().ptr)
			{
				return this->first->atomConf() < other.first->atomConf();
			}

			return this->first->desc() < other.first->desc();
		}
		else
		{
			return this->second > other.second;
		}
	}
};

inline std::ostream &operator<<(std::ostream &ss, const ProbeTypePair &ptp)
{
	if (ptp.first)
	{
		ss << ptp.first->desc();
		switch (ptp.second)
		{
			case hnet::BondType: ss << " bonding"; break;
			case hnet::ChargeType: ss << " charge"; break;
			default: ss << " existence"; break;
		}
	}
	return ss;
}


struct OneProbe
{
	Probe *probe;
	hnet::Types type;
	int value;
};

class ProbeResult
{
public:
	ProbeResult(const std::vector<OneProbe> &results, const GetScore &score)
	{
		_results = results;
		_score = score;
	}
	
	const std::vector<OneProbe> &results() const
	{
		return _results;
	}

	const void addResult(const OneProbe &result)
	{
		_results.push_back(result);
	}

	const GetScore &getScore() const
	{
		return _score;
	}
	
private:
	std::vector<OneProbe> _results;
	GetScore _score;
};

#endif
