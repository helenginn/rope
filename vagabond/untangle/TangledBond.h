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

#ifndef __vagabond__TangledBond__
#define __vagabond__TangledBond__

#include <string>

class BondLength;
class Atom;

class TangledBond
{
public:
	TangledBond(BondLength &src);

	operator BondLength &()
	{
		return _src;
	}
	
	Atom *atom(int i);
	std::vector<std::pair<std::string, std::string>> options();
	
	float total_score(float bias);
	float simple_score(const std::string &q, const std::string &r);
	float length_score(const std::string &p, const std::string &q, 
	                   const std::string &r, const std::string &s);
	float angle_score(const std::string &p, const std::string &q, 
	                  const std::string &r, const std::string &s);
	
	float volatility();
	
	bool hasHydrogens() const;
	
	std::string desc() const;
private:

	BondLength &_src;

};

#endif
