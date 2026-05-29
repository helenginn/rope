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

#include "Symmetry.h"
#include <iostream>

Symmetry::Symmetry()
{
}

void Symmetry::applyTransform(const glm::mat4x4 &which, const NextPoint &next_point)
{
	glm::mat4x4 undo = which * glm::inverse(_applied);
	while (true)
	{
		glm::vec3 *ptr = next_point();
		if (!ptr)
		{
			break;
		}
		glm::vec3 update = glm::vec3(undo * glm::vec4(*ptr, 1.f));
		*ptr = update;
	}
	_applied = undo;
}

float Symmetry::similarity_score(const glm::mat4x4 &left, 
                                 const glm::mat4x4 &right)
{
	glm::mat4x4 undo = glm::inverse(right) * left;
	float trace = 3 - (undo[0][0] + undo[1][1] + undo[2][2]);
	float trans = glm::length(glm::vec3(undo[3]));
	float result = (1 + trace) * (1 + trans) - 1;
	return result;
}

bool Symmetry::add_if_new(const glm::mat4x4 &new_mat)
{
	bool found = false;
	for (const glm::mat4x4 &old : _transforms)
	{
		float score = similarity_score(old, new_mat);
		if (score < _threshold)
		{
			found = true; break;
		}
	}

	if (!found)
	{
		_transforms.push_back(new_mat);
	}

	return !found;
}

Symmetry::NextPoint Symmetry::next_pointer(glm::vec3 *vec)
{
	struct next_point
	{
		next_point(glm::vec3 *vec)
		{
			v = vec;
		}
		
		glm::vec3 *operator()()
		{
			glm::vec3 *tmp = v;
			v = nullptr;
			return tmp;
		}

        glm::vec3 *v{};
	};
	
	return next_point(vec);
}
