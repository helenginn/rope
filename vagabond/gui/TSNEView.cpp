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

#include "TSNEView.h"
#include <vagabond/c4x/ClusterTSNE.h>

TSNEView::TSNEView() : PointyView()
{
	setName("TSNE View");

}

void TSNEView::updatePoints()
{
	for (size_t i = vertexCount(); i < _cluster->pointCount(); i++)
	{
		glm::vec3 v = _cluster->pointForDisplay(i);
		addPoint(v, 0);
	}
	
	if (vertexCount() > _cluster->pointCount())
	{
		_vertices.resize(_cluster->pointCount());
	}

	for (size_t i = 0; i < vertexCount(); i++)
	{
		glm::vec3 v = _cluster->pointForDisplay(i);
		_vertices[i].pos = v;
	}
}

void TSNEView::makePoints()
{
	if (_cluster == nullptr)
	{
		return;
	}

	clearVertices();
	
	size_t count = _cluster->pointCount();
	_vertices.reserve(count);
	_indices.reserve(count);

	for (size_t i = 0; i < count; i++)
	{
		glm::vec3 v = _cluster->pointForDisplay(i);
		addPoint(v, 0);
	}
	
	reindex();
	forceRender();
}
