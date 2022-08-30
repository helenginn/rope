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

#include "PlaneView.h"
#include "PlaneGrid.h"

PlaneView::PlaneView(Cluster<MetadataGroup> *cluster, Molecule *m) : Renderable()
{
	setName("Plane view");
	_renderType = GL_TRIANGLES;
	Plane *p = new Plane(m, cluster);
	_plane = p;

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");
}

PlaneView::PlaneView(Plane *p) : Renderable()
{
	_plane = p;
}

void PlaneView::recolour()
{
	int nx = _plane->num(0);
	int ny = _plane->num(1);
	float ref = _plane->mean();
	float first = _plane->reference();
	float stdev = _plane->stdev();
	
	if (stdev > 1)
	{
		stdev = 1;
	}
	
	std::cout << "First: " << first << std::endl;
	std::cout << "Mean: " << ref << std::endl;
	std::cout << "Stdev: " << stdev << std::endl;

	for (int y = 0; y < ny; y++)
	{
		for (int x = 0; x < nx; x++)
		{
			int idx = _plane->toIndex(x, y);
			float score = _plane->score(idx);
			score -= first;
//			score /= stdev;

			Vertex &v = _vertices[idx];
			val_to_cluster4x_colour(score, &v.color[0], &v.color[1], &v.color[2]);
			v.color /= 255.f;
			v.color[3] = 1.f;

		}
	}

	setAlpha(0.4);
}

void PlaneView::makeGrid()
{
	PlaneGrid *pg = new PlaneGrid(_plane);
	pg->populate();
	_pg = pg;
}

void PlaneView::render(SnowGL *gl)
{
	_pg->render(gl);
	Renderable::render(gl);
}

void PlaneView::populate()
{
	deleteTemps();
	makeGrid();

	clearVertices();

	int nx = _plane->num(0);
	int ny = _plane->num(1);
	
	bool connect_left = false;
	bool connect_top = false;

	for (int y = 0; y < ny; y++)
	{
		connect_left = false;

		for (int x = 0; x < nx; x++)
		{
			glm::vec3 v = _plane->generateVertex(x, y);
			addVertex(v);
			
			if (connect_left && connect_top)
			{
				addIndex(-ny - 2);
				addIndex(-ny - 1);
				addIndex(-2);
				addIndex(-ny - 1);
				addIndex(-2);
				addIndex(-1);
			}
			
			connect_left = true;
		}
		
		connect_top = true;
	}
	
	recolour();
	rebufferVertexData();
	rebufferIndexData();
}

void PlaneView::respond()
{
	recolour();
	forceRender(true, false);
}
