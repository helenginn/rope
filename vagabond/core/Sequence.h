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

#ifndef __vagabond__Sequence__
#define __vagabond__Sequence__

#include <vector>
#include <map>
#include "Residue.h"

class Atom;

class Sequence
{
public:
	Sequence(Atom *anchor);
	Sequence();
	Sequence(std::string str);
	
	Sequence &operator+=(Sequence *&other);
	Sequence &operator+=(Residue &res);

	size_t size()
	{
		return _residues.size();
	}
	
	Residue *residue(int i)
	{
		return &_residues[i];
	}
	
	int firstNum()
	{
		if (_residues.size() == 0)
		{
			return 0;
		}

		return _residues[0].as_num();
	}
	
	int lastNum()
	{
		if (_residues.size() == 0)
		{
			return 0;
		}

		return _residues.back().as_num();
	}
	
	std::string str();

	friend void to_json(json &j, const Sequence &value);
	friend void from_json(const json &j, Sequence &value);
private:
	void findSequence();
	
	std::vector<Residue> _residues;
	Atom *_anchor;
};

/* sequence */
inline void to_json(json &j, const Sequence &value)
{
	j["residues"] = value._residues;
}

/* sequence */
inline void from_json(const json &j, Sequence &value)
{
	value._residues = j.at("residues");
}

#endif
