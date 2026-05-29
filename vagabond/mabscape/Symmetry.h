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

#ifndef __vagabond__Symmetry__
#define __vagabond__Symmetry__

#include <vagabond/utils/glm_import.h>
#include <functional>

class Symmetry
{
public:
	Symmetry();

	typedef std::function<glm::vec3 *()> NextPoint;
	void applyTransform(const glm::mat4x4 &which, const NextPoint &next_point);
	
	static NextPoint next_pointer(glm::vec3 *v);
	
	const std::vector<glm::mat4x4> &transforms() const
	{
		return _transforms;
	}

	float similarity_score(const glm::mat4x4 &left, const glm::mat4x4 &right);

	const glm::vec3 &reference() const
	{
		return _reference;
	}
	
	bool add_if_new(const glm::mat4x4 &new_mat);
protected:
	std::vector<glm::mat4x4> _transforms;

	glm::mat4x4 _applied = glm::mat4{1.f};
	glm::vec3 _reference{};

	float _threshold{10.f};
};

#endif
