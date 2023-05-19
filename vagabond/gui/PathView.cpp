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

#include "PathView.h"
#include <vagabond/core/MetadataGroup.h>
#include <vagabond/core/Polymer.h>
#include <vagabond/core/Path.h>
#include <vagabond/core/Trajectory.h>
#include <vagabond/c4x/ClusterSVD.h>

PathView::PathView(Path &path, ClusterSVD<MetadataGroup> *cluster) 
: SimplePolygon(), _path(path)
{
	setName("Path view");
	_renderType = GL_LINES;

	_cluster = cluster;

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");
}

PathView::~PathView()
{

}

void PathView::populate()
{
	MetadataGroup *dg = static_cast<MetadataGroup *>(_cluster->objectGroup());

	Trajectory *traj = _path.calculateTrajectory(16);
	const MetadataGroup::Array &average = dg->average();
	
	clearVertices();

	for (size_t i = 0; i < traj->size(); i++)
	{
		MetadataGroup::Array angles = traj->nakedTrajectory(i).storage_only();
		dg->convertToDifferences(angles, &average);

		std::vector<float> mapped = _cluster->mapVector(angles);
		glm::vec3 point = _cluster->pointForDisplay(mapped);
		
		Vertex &v = addVertex(point);
		
		if (i > 0)
		{
			addIndex(-2);
			addIndex(-1);
		}
	}
	
	int idx = dg->indexOfObject(_path.endInstance());
	glm::vec3 end = _cluster->point(idx);
	_cluster->reweight(end);
	_version = _cluster->version();

	addVertex(end);
	addIndex(-2);
	addIndex(-1);

	setAlpha(1.f);
	
	std::cout << "Loaded " << _path.id() << std::endl;
	forceRender(true, true);
	_populated = true;
}

bool PathView::needsUpdate()
{
	return (_version < _cluster->version() || !isPopulated());
}
