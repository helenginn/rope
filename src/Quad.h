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

#include "Renderable.h"

class Quad : public Renderable
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
	
	void setTexture(int i, GLuint val)
	{
		_textures[i] = val;
	}

	virtual void render(SnowGL *sender);
protected:
	virtual void extraUniforms();
	virtual void bindTextures();

private:
	void prepareNormalDist();
	void makeQuad();

	GLint _mode;
	GLfloat _threshold;
	GLfloat _other;
	GLfloat _dist[20];
	int _count;
};

#endif
