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

#ifndef __vagabond__ParamSet__
#define __vagabond__ParamSet__

#include <vagabond/utils/OpSet.h>
#include <sstream>

class Parameter;

class ParamSet : public OpSet<Parameter *>
{
public:
	ParamSet(const std::set<Parameter *> &other) : OpSet(other) {}
	ParamSet(Parameter *const &member) : OpSet(member) {}
	ParamSet() : OpSet() {}
	
	friend std::ostream &operator<<(std::ostream &ss, const ParamSet &set);

	void expandNeighbours();
	void excludeBeyond(int residue_num, int direction);
	ParamSet terminalSubset() const;
	
	std::vector<Parameter *> toVector();

	template <class Filter>
	void filter(const Filter &filter)
	{
		ParamSet filtered;

		for (Parameter *p : *this)
		{
			if (filter(p))
			{
				filtered.insert(p);
			}
		}

		*this = filtered;
	}

private:

};

#endif
