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
#include <vagabond/c4x/Cluster.h>

PathView::PathView(Cluster<MetadataGroup> *cluster, Molecule *mol)
{
	setName("Path view");
	_renderType = GL_LINES;
	PathFinder *pf = new PathFinder(mol, cluster, 20);
	_pathFinder = pf;
	_cluster = cluster;
	_molecule = mol;

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");
}

PathView::~PathView()
{

}

void PathView::start()
{
	for (size_t i = 0; i < _pathFinder->dims(); i++)
	{
		std::vector<ResidueTorsion> list = _cluster->dataGroup()->headers();
		std::vector<float> vals = _cluster->torsionVector(i);
		
		_pathFinder->addAxis(list, vals);
	}
	
	if (_pathFinder->end())
	{
		std::cout << _pathFinder->begin()->id() << " to " << 
		_pathFinder->end()->id() << std::endl;
	}
	
	_pathFinder->setResponder(this);
	_pathFinder->setup();
	_worker = new std::thread(&PathFinder::start, _pathFinder);
}

void PathView::addNewPoints()
{
	size_t existing = _vertices.size();
	size_t full = _pathFinder->nodeCount();
	int idx = _cluster->dataGroup()->indexOfObject(_molecule);
	glm::vec3 centre = _cluster->point(idx);

	for (size_t i = existing; i < full; i++)
	{
		std::vector<float> placement = _pathFinder->placement(i);
		glm::vec3 pos = _cluster->point(placement);
		Vertex &v = addVertex(pos);
		v.pos += centre;

		float score = _pathFinder->score(i);
		if (existing == 0)
		{
			_first = score;
		}

		score -= _first;
		val_to_cluster4x_colour(score, &v.color[0], &v.color[1], &v.color[2]);
		v.color /= 255.f;
		v.color[3] = 1.f;
		
		int parent = _pathFinder->parentIndex(i);

		if (parent == -1)
		{
			continue;
		}
		
		addIndex(parent);
		addIndex(-1);
	}
}

void PathView::respond()
{
	addNewPoints();
	forceRender(true, true);
}

