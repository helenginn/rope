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
#ifndef __vagabond__Contacts__
#define __vagabond__Contacts__

#include <map>
#include <mutex>
#include "../ResidueId.h"

struct Contacts
{
	typedef std::map<ResidueId, std::map<ResidueId, float>> Map;
	static std::mutex *mutex;
	
	~Contacts()
	{
	}
	
	Map totals;

	float operator()()
	{
		float sum = 0;
		for (auto it = totals.begin(); it != totals.end(); it++)
		{
			for (auto jt = it->second.begin(); jt != it->second.end(); jt++)
			{
				sum += jt->second;
				if (it->first == jt->first)
				{
					sum += jt->second;
				}
			}
		}
		return sum / 2;
	}

	std::map<ResidueId, float> &operator[](const ResidueId &id)
	{
		return totals[id];
	}
	
	void operator=(const Contacts &c)
	{
		std::unique_lock<std::mutex> lock(*mutex);

		for (auto it = c.totals.begin(); it != c.totals.end(); it++)
		{
			for (auto jt = it->second.begin(); jt != it->second.end(); jt++)
			{
				float score = jt->second;
				totals[it->first][jt->first] += score;
				totals[jt->first][it->first] += score;
			}
		}
	}
};

#endif
