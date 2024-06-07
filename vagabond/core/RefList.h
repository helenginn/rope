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

class Diffraction;

/** \class RefList
 *  This class stores reflections in a list format (rather than inserted
 *  onto a grid) and covers symmetry operation work on the list.
 **/

class RefList
{
public:
	RefList(const std::vector<Reflection> &refls);
	~RefList();
	
	void extractSymops();
	
	size_t symOpCount()
	{
		return _nsymops;
	}
	
	void setUnitCell(std::array<double, 6> &cell);
	const std::array<double, 6> &unitCell()
	{
		return _cell;
	}
	
	const double resolutionOf(const int idx) const;
	const double resolutionOf(glm::vec3 v) const;
	const glm::vec3 reflAsFraction(const int idx) const;
	const glm::vec3 reflAsIndex(const int idx) const;

	const size_t reflectionCount() const
	{
		return _refls.size();
	}
	
	const Reflection &operator[](int i) const
	{
		return _refls.at(i);
	}
	
	void addReflectionToGrid(Diffraction *diff, int i);

    Reflection::HKL maxSymHKL();
	
	const glm::mat3x3 &frac2Real() const
	{
		return _frac2Real;
	}
	
	void setSpaceGroupName(const std::string &name)
	{
		_spgName = name;
	}
	
	std::string spaceGroupName()
	{
		return _spgName;
	}
private:
	std::vector<Reflection> _refls;
	std::array<double, 6> _cell;

	glm::mat3x3 _frac2Real = glm::mat3(1.f);
	glm::mat3x3 _recip2Frac = glm::mat3(1.f);
	
	size_t _nsymops = 0;
	
	std::string _spgName;
};

#endif
