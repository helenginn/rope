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

#ifndef __vagabond__ResidueContacts__
#define __vagabond__ResidueContacts__

#include <mutex>

struct Contacts
{
	Eigen::MatrixXf matrix;

	auto &operator()(const ResidueId &x, const ResidueId &y) 
	{
		if (map.count(x) && map.count(y))
		{
			return matrix(map[x], map[y]);
		}

		return _fail;
	}

	void setup(const std::set<ResidueId> &residues)
	{
		resis = OpVec<ResidueId>(residues);
		matrix = Eigen::MatrixXf(residues.size(), residues.size());
		clear();

		int i = 0;
		for (const ResidueId &id : resis)
		{
			map[id] = i;
			i++;
		}
	}
	
	OpSet<OpSet<ResidueId>> grouped()
	{
		OpSet<ResidueId> full(resis);
		OpSet<OpSet<ResidueId>> groups;

		while (full.size() > 0)
		{
			const ResidueId id = *full.begin();
			full.erase(full.begin());
			OpSet<ResidueId> group = {id};

			while (full.size() > 0)
			{
				bool untouched = true;
				for (const ResidueId &candidate : full)
				{
					bool found = false;
					for (const ResidueId &already : group) 
					{
						float val = (*this)(already, candidate);
						if (val > 10)
						{
							found = true;
							untouched = false;
							break;
						}
					}

					if (found)
					{
						group.insert(candidate);
						full.erase(candidate);
						break;
					}
				}
				
				if (untouched)
				{
					std::cout << group.size() << " ";
					groups.insert(group);
					break;
				}
			}
		}

		std::cout << std::endl;
		return groups;
	}

	void clear()
	{
		matrix.setZero();
	}

	float _fail = -1;
	std::map<ResidueId, int> map;
	OpVec<ResidueId> resis;
	std::mutex mutex;
};


#endif
