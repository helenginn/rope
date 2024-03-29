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

#ifndef __vagabond__Vertex__
#define __vagabond__Vertex__

#include <vagabond/utils/glm_import.h>
#include <iostream>

namespace Snow
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec4 color;
		glm::vec4 extra;
		glm::vec2 tex;
	};
	
}

using namespace Snow;

inline std::ostream &operator<<(std::ostream &ss, const Vertex &v)
{
	ss << "Pos: " << v.pos << std::endl;
	ss << "Normal: " << v.normal << std::endl;
	ss << "Color: " << v.color << std::endl;
	ss << "Extra: " << v.extra << std::endl;
	ss << "Tex: " << v.tex << std::endl;
	return ss;
}


#endif
