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

#include "StarView.h"
#include <vagabond/utils/maths.h>
#include <vagabond/core/RopeCluster.h>
#include <vagabond/core/Path.h>
#include <vagabond/gui/elements/Line.h>

StarView::StarView()
{
	setName("Star view");
	_line = new Line();
	addObject(_line);
}

void StarView::setPaths(std::set<Path *> &paths)
{
	for (Path *path : _paths)
	{
		delete path;
	}

	_paths.clear();

	for (Path *path : paths)
	{
		_paths.insert(new Path(*path));
	}
}

void StarView::updatePoints()
{

}

void StarView::colourVertex(Vertex &v)
{
	glm::vec3 hsv = glm::vec3(hue * 360, 100, 80);
	glm::vec3 rgb = hsv;
	hsv_to_rgb(rgb.r, rgb.g, rgb.b);

	v.color = glm::vec4(rgb, 1.0);
}

void StarView::incrementColour()
{
	hue += 1.61;
	
	while (hue > 1)
	{
		hue -= 1;
	}
}

void StarView::makePoints()
{
	if (_cluster == nullptr)
	{
		return;
	}

	clearVertices();
	_line->clearVertices();
	
	size_t count = _cluster->pointCount();
	MetadataGroup *dg = static_cast<MetadataGroup *>(_cluster->objectGroup());

	_vertices.reserve(count);
	_indices.reserve(count);
	addPoint(glm::vec3{}, 0);
	
	// first vertex is 0,0,0 for drawing lines

	for (Path *path : _paths)
	{
		Vertex &vert = _line->addPoint(glm::vec3{0});
		colourVertex(vert);
		path->setStepCount(_steps);
		path->calculateDeviations(dg, false);

		for (size_t i = 0; i < path->angleArraySize(); i++)
		{
			MetadataGroup::Array angles = path->deviation(i);
			
			std::vector<float> mapped = _cluster->mapVector(angles);
			glm::vec3 point = _cluster->pointForDisplay(mapped);
			
			float l = glm::length(point);

			Vertex &vert = _line->addPoint(point);
			colourVertex(vert);
		}

		Vertex &wert = _line->addPoint(glm::vec3{0});
		colourVertex(wert);
		incrementColour();
	}
	
	reindex();
	forceRender();
	_line->forceRender();
}

void StarView::setCluster(TorsionCluster *cluster)
{
	_cluster = cluster;
}
