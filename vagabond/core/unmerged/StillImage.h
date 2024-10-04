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

#ifndef __vagabond__StillImage__
#define __vagabond__StillImage__

#include <vector>
#include <set>
#include "Locality.h"

class Locality;

class StillImage : public std::set<Obs>
{
public:
	StillImage() {};
	StillImage(char *&str_ptr);

	float cc(const StillImage &other);
	int number_in_common(const StillImage &other);
	
	std::vector<Locality> localities() const;
	
	int number()
	{
		return _crystal;
	}
private:
	int checkLine(char *&str_ptr);
	int _crystal = -1;
	
	bool _flagged = false;
};

#endif
