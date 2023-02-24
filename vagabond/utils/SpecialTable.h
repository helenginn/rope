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

#ifndef __vagabond__SpecialTable__
#define __vagabond__SpecialTable__

#include "glm_import.h"

class SpecialTable
{
public:
	void toAmpOffset(float x, float y, float *amp, float *offset);
	void toTorsions(float *x, float *y, float amp, float offset);
private:
	glm::vec2 _origin = glm::vec2(-71.4, 0.0);
	glm::vec2 _a0 = glm::vec2(-85.44, 21.29);
	glm::vec2 _b0 = glm::vec2(-103.4, -22.44);
	glm::vec2 _amps = glm::vec2(0.771, 0.7925);
	float _offset = -0.1082;

};

#endif
