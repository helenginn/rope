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

#include "ClusterView.h"
#include "ColourScheme.h"
#include <vagabond/core/Rule.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/HasMetadata.h>
#include <vagabond/core/MetadataGroup.h>
#include <iostream>

ClusterView::ClusterView()
{
	setUsesProjection(true);
	_renderType = GL_POINTS;
	setFragmentShaderFile("assets/shaders/point.fsh");
	setVertexShaderFile("assets/shaders/point.vsh");
	setImage("assets/images/dot.png");
}

void ClusterView::makePoints()
{
	clearVertices();
	
	size_t count = _cx->pointCount();
	_vertices.reserve(count);
	_indices.reserve(count);

	for (size_t i = 0; i < count; i++)
	{
		glm::vec3 v = _cx->point(i);
		v *= 10;
		Snow::Vertex &vert = addVertex(v);
		vert.color[3] = 1;
		addIndex(-1);
	}
}

void ClusterView::setCluster(Cluster<MetadataGroup> *cx)
{
	_cx = cx;

	makePoints();
}

void ClusterView::extraUniforms()
{
	const char *uniform_name = "size";
	GLuint u = glGetUniformLocation(_usingProgram, uniform_name);
	glUniform1f(u, _size);
	checkErrors("rebinding size");
}

void ClusterView::applyVaryColour(const Rule &r)
{
	std::string header = r.header();
	Scheme sch = r.scheme();
	ColourScheme *cs = new ColourScheme(sch);
	
	if (cs == nullptr)
	{
		return;
	}

	MetadataGroup &group = *_cx->dataGroup();
	for (size_t i = 0; i < group.objectCount(); i++)
	{
		HasMetadata *obj = group.object(i);

		const Metadata::KeyValues *data = group.object(i)->metadata();
		
		if (data == nullptr || data->count(header) == 0 
		    || !data->at(header).hasNumber())
		{
			_vertices[i].color = glm::vec4(0.5, 0.5, 0.5, 0.2);
			continue;
		}
		
		float val = data->at(header).number();
		r.convert_value(val);
		glm::vec4 colour = cs->colour(val);
		_vertices[i].color = colour;
	}
	
	forceRender();

	delete cs;
}

void ClusterView::applyRule(const Rule &r)
{
	if (r.type() == Rule::VaryColour)
	{
		applyVaryColour(r);
	}
	
}

