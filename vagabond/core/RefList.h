// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__RefList__
#define __vagabond__RefList__

#include "Reflection.h"
#include "../utils/glm_import.h"
#include <vector>
#include <array>

class RefList
{
public:
	RefList(std::vector<Reflection> &refls);
	
	void setUnitCell(std::array<double, 6> &cell);
	
	const double resolutionOf(const int idx) const;

	const size_t reflectionCount() const
	{
		return _refls.size();
	}
	
	HKL maxHKL();
private:
	std::vector<Reflection> _refls;
	std::array<double, 6> _cell;

	glm::mat3x3 _frac2Real = glm::mat3(1.f);
	glm::mat3x3 _recip2Frac = glm::mat3(1.f);
};

#endif
