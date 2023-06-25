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

#ifndef __vagabond__PointyView__
#define __vagabond__PointyView__

#include <vagabond/gui/elements/IndexResponder.h>

#define POINT_TYPE_COUNT 8

class PointyView : public IndexResponder 
{
public:
	PointyView();
	
	void refresh();

	void addPoint(glm::vec3 pos, int pointType);
	void setPointType(int idx, int type);
	
	const size_t pointTypeCount() const
	{
		return POINT_TYPE_COUNT;
	}

	virtual bool index_in_range(int idx, float t, float l, float b, float r);

	virtual size_t requestedIndices()
	{
		return _vertices.size();
	}

	void reset();
	virtual void reindex();

	virtual bool mouseOver();
	virtual void unMouseOver();

	virtual void makePoints() = 0;
	virtual void updatePoints() = 0;
	virtual void additionalJobs() {};
protected:
	virtual void extraUniforms();
	void customiseTexture(Snow::Vertex &vert);

	float _size = 40;
private:

};

#endif
