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

#ifndef __vagabond__TransferHandler__
#define __vagabond__TransferHandler__

#include "engine/Handler.h"

class Atom;

class TransferHandler : public Handler
{
public:
	TransferHandler(int mapNum);

	void supplyAtomGroup(const std::vector<Atom *> &all);
	
	/** set length dimension of cubic voxel
	 * 	@param dim length in Angstroms */
	void setCubeDim(float dim)
	{
		_cubeDim = dim;
	}
protected:
	void getRealDimensions(const std::vector<Atom *> &sub);

	float _cubeDim = 0.6;
	int _mapNum = 2;
	
	glm::vec3 _min = glm::vec3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	glm::vec3 _max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	glm::vec3 _pad = glm::vec3(2, 2, 2);

};

#endif
