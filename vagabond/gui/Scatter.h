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

#ifndef __vagabond__Scatter__
#define __vagabond__Scatter__

#include <vagabond/gui/elements/IndexResponder.h>

class Graph;

class Scatter : public IndexResponder
{
public:
	Scatter(Graph *graph, int index);
	
	virtual ~Scatter();

	virtual size_t requestedIndices()
	{
		return vertexCount();
	}

	virtual void reindex();

	void addPoint(glm::vec3 vec, glm::vec3 colour, int pointType,
	              float alpha = 1.f);

	// overwrites vertex idx's colour (alpha untouched) without uploading -
	// call refreshColour() once after any number of these to actually
	// reflect the change on screen. Used by Graph::hoverColour().
	void setPointColour(size_t idx, glm::vec3 colour);
	void setAllColour(glm::vec3 colour);
	void refreshColour();

	virtual void interacted(int idx, bool hover, bool left);
	virtual void extraUniforms();
private:
	Graph *_graph{};
	int _index{};
	float _size = 40;

};

#endif
