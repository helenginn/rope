// helen3d
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

#ifndef __Quad__Quad__
#define __Quad__Quad__

#include "Box.h"

class Quad : public Box
{
public:
	Quad();
	
	void setMode(int mode)
	{
		_mode = mode;
	}
	
	void setHorizontal(int mode)
	{
		_mode = mode;
	}
	
	void clearThreshold()
	{
		_threshold = 0;
	}
	
	void addToThreshold(float val = 1)
	{
		_threshold += val;
	}
	
	void setOther(float other)
	{
		_other = other;
	}

	void setThreshold(float thresh)
	{
		_threshold = thresh;
	}

	void prepareTextures(SnowGL *sender);
	
	void setTextureID(int idx, GLuint val)
	{
		if (_textures.size() <= idx)
		{
			_textures.resize(idx + 1);
		}

		_textures[idx] = val;
	}

	virtual void render(SnowGL *sender);
protected:
	virtual void extraUniforms();
	virtual void bindTextures();

private:
	void prepareNormalDist();
	
	std::vector<GLuint> _textures;
	GLint _mode = 3;
	GLfloat _threshold;
	GLfloat _other;
	GLfloat _dist[20];

	int _count;
};

#endif
