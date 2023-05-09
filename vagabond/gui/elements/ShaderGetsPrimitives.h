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

#ifndef __vagabond__ShaderGetsPrimitives__
#define __vagabond__ShaderGetsPrimitives__

#include "ShaderGetsVertices.h"
#include <map>

class ShaderGetsPrimitives : public ShaderGetsVertices
{
public:
	ShaderGetsPrimitives(Renderable *r);
	virtual ~ShaderGetsPrimitives() {}

	virtual void prepareBuffers();

	virtual void enablePointers();

	virtual void extraVariables();
	virtual void setupExtraVariables();

	virtual size_t vSize();
	virtual void *vPointer();

	virtual size_t iSize();
	virtual void *iPointer();
private:
	struct Primitive
	{
		glm::vec3 pos; // 0
		glm::vec3 second; // 0 + 3 
		glm::vec3 third; // 0 + 3 + 3
		glm::vec3 normal; // 0 + 3 + 3 + 3
		glm::vec4 color; // 0 + 3 + 3 + 3 + 3

	};

	friend std::ostream &operator<<(std::ostream &ss, const Primitive &p)
	{
		ss << "first: " << p.pos << std::endl;
		ss << "second: " << p.second << std::endl;
		ss << "third: " << p.third << std::endl;
		return ss;
	}

	std::vector<Primitive> _primitives;
	std::vector<GLuint> _triIndices;
};

#endif
