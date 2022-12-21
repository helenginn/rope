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
#include <vagabond/core/Molecule.h>
#include <vagabond/core/Path.h>
#include <vagabond/c4x/ClusterSVD.h>

PathView::PathView(Path &path, ClusterSVD<MetadataGroup> *cluster) : _path(path)
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

void PathView::render(SnowGL *gl)
{
	Renderable::render(gl);
}

void PathView::populate()
{
	MetadataGroup *dg = static_cast<MetadataGroup *>(_cluster->objectGroup());

	if (_path.angleArraySize() == 0)
	{
		_path.calculateArrays(dg);
	}

	const MetadataGroup::Array &average = dg->average();
	
	for (size_t i = 0; i < _path.angleArraySize(); i++)
	{
		MetadataGroup::Array angles = _path.angleArray(i);

		dg->convertToDifferences(angles, &average);

		std::vector<float> mapped = _cluster->mapVector(angles);
		glm::vec3 point = _cluster->point(mapped);
		
		addVertex(point);
		
		if (i > 0)
		{
			addIndex(-2);
			addIndex(-1);
		}
	}
	
	int idx = dg->indexOfObject(_path.endMolecule());
	glm::vec3 end = _cluster->point(idx);

	addVertex(end);
	addIndex(-2);
	addIndex(-1);

	setAlpha(1.f);
	
	std::cout << "Loaded " << _path.id() << std::endl;
	forceRender(true, true);
}

