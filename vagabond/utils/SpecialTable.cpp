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

#include "SpecialTable.h"
#include <iostream>

void SpecialTable::toAmpOffset(float x, float y, float *amp, float *offset)
{
	glm::vec2 op = glm::vec2(x, y);
	op -= _origin;
	
	glm::mat2x2 mat = glm::mat2x2((_a0 - _origin), (_b0 - _origin));
	glm::mat2x2 conv2Abstract = glm::inverse(mat);

	glm::vec2 abstr = conv2Abstract * op;

	glm::vec2 scaled = glm::vec2(abstr.x * _amps.x, abstr.y * _amps.y);
	float l = glm::length(scaled);
	*amp = l;
	
	abstr = glm::normalize(abstr);

	*offset = -atan2(-abstr.y, abstr.x);
	if (*offset < 0) *offset += 2 * M_PI;
	*offset /= 2 * M_PI;
	 *offset -= _offset;
	*offset /= 2;
	if (*offset != *offset)
	{
		*offset = 0.f;
	}
}

void SpecialTable::toTorsions(float *x, float *y, float amp, float offset)
{
	offset *= 2;
	offset += _offset;
	offset *= 2 * M_PI;
	
	glm::vec2 abstr = glm::vec2(cos(offset), sin(offset));
	abstr *= amp;
	abstr.x /= _amps.x;
	abstr.y /= _amps.y;

	glm::mat2x2 mat = glm::mat2x2((_a0 - _origin), (_b0 - _origin));
	
	glm::vec2 op = mat * abstr;
	op += _origin;
	*x = op.x;
	*y = op.y;
}
