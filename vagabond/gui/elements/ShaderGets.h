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

#ifndef __vagabond__ShaderGets__
#define __vagabond__ShaderGets__

#include "Vertex.h"
#include <vagabond/utils/gl_import.h>
#include <vector>

class Renderable;

class ShaderGets
{
public:
	ShaderGets(Renderable *parent)
	{
		_parent = parent;
	}

	virtual ~ShaderGets() {};
	virtual void setupVBOBuffers() = 0;

	virtual void prepareBuffers() = 0;

	virtual void enablePointers() = 0;

	virtual void extraVariables() = 0;
	virtual void setupExtraVariables() = 0;

	virtual void rebufferVertexData() = 0;
	virtual void rebufferIndexData() = 0;

	virtual size_t vSize() = 0;
	virtual void *vPointer() = 0;

	virtual size_t iSize() = 0;
	virtual void *iPointer() = 0;
protected:
	Renderable *_parent = nullptr;

};

#endif
