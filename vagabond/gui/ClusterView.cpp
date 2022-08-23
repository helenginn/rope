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
#include "ConfSpaceView.h"
#include <vagabond/gui/elements/SnowGL.h>
#include <vagabond/gui/elements/FloatingText.h>

#include <vagabond/core/Rule.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Molecule.h>
#include <vagabond/core/MetadataGroup.h>
#include <iostream>

ClusterView::ClusterView()
{
	setUsesProjection(true);
	setName("ClusterView");
	_renderType = GL_POINTS;
	setFragmentShaderFile("assets/shaders/point.fsh");
	setVertexShaderFile("assets/shaders/point.vsh");
	setImage("assets/images/points.png");
	
#ifdef __EMSCRIPTEN__
	setSelectable(true);
#endif
}

ClusterView::~ClusterView()
{
	deleteObjects();
}

void ClusterView::customiseTexture(Snow::Vertex &vert)
{
	vert.tex.x = pointTypeCount(); /* number of points */
	vert.tex.y = 0.; /* point index */
	vert.color[3] = 1;
}

void ClusterView::setPointType(int idx, int type)
{
	_vertices[idx].tex.y = type; /* point index */
}

void ClusterView::addPoint(glm::vec3 pos, int pointType)
{
	Snow::Vertex &vert = addVertex(pos);
	customiseTexture(vert);
	setPointType(_vertices.size() - 1, pointType);

	addIndex(-1);
}

void ClusterView::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < vertexCount(); i++)
	{
		/* in the case of multiple responders */
		_vertices[i].extra[0] = i + offset + 1.5;
	}
}

void ClusterView::reset()
{
	for (size_t i = 0; i < vertexCount(); i++)
	{
		Snow::Vertex &vert = _vertices[i];
		vert.tex.y = 0.; /* point index */
		vert.color = glm::vec4(0., 0., 0., 1.);
	}
}

void ClusterView::refresh()
{
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 v = _cx->point(i);
		_vertices[i].pos = v;
	}
	
	rebufferVertexData();

}

void ClusterView::prioritiseMetadata(std::string key)
{
	std::vector<float> vals = _cx->dataGroup()->numbersForKey(key);
//	int idx = _cx->bestAxisFit(vals);

	refresh();
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
		addPoint(v, 0);
	}
	
	reindex();
}

void ClusterView::render(SnowGL *gl)
{
	Renderable::render(gl);
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
		const Metadata::KeyValues data = group.object(i)->metadata();
		
		if (data.count(header) == 0 || !data.at(header).hasNumber())
		{
			_vertices[i].color = glm::vec4(0.5, 0.5, 0.5, 0.2);
			continue;
		}
		float val = data.at(header).number();
		r.convert_value(val);
		glm::vec4 colour = cs->colour(val);
		_vertices[i].color = colour;
	}
	
	forceRender();

	delete cs;
}

void ClusterView::applyChangeIcon(const Rule &r)
{
	std::string header = r.header();
	std::string value = r.headerValue();
	int pt = r.pointType();
	bool any_assigned = r.ifAssigned();

	MetadataGroup &group = *_cx->dataGroup();
	for (size_t i = 0; i < group.objectCount(); i++)
	{
		const Metadata::KeyValues data = group.object(i)->metadata();
		
		if (data.count(header) == 0)
		{
			continue;
		}
		
		if (data.count(header) && any_assigned)
		{
			setPointType(i, pt);
		}
		else if (data.at(header).text() == value)
		{
			setPointType(i, pt);
		}
	}
	
	forceRender();
}

void ClusterView::applyRule(const Rule &r)
{
	if (r.type() == Rule::VaryColour)
	{
		applyVaryColour(r);
	}
	else if (r.type() == Rule::ChangeIcon)
	{
		applyChangeIcon(r);
	}
}

void ClusterView::click(bool left)
{
	interacted(currentVertex(), false, left);
}

bool ClusterView::mouseOver()
{
	interacted(currentVertex(), true);
	return (currentVertex() >= 0);
}

void ClusterView::unMouseOver()
{
	interacted(-1, true);
}

void ClusterView::interacted(int idx, bool hover, bool left)
{
	if (_text != nullptr)
	{
		removeObject(_text);
		delete _text;
		_text = nullptr;
	}
	
	if (idx < 0 || idx >= _vertices.size())
	{
		return;
	}
	
	MetadataGroup &group = *_cx->dataGroup();

	if (_confSpaceView->returnToView() && left && !hover)
	{
		Molecule *mol = static_cast<Molecule *>(group.object(idx));
		_confSpaceView->reorientToMolecule(mol);
		return;
	}

	std::string str = group.object(idx)->id();

	FloatingText *ft = new FloatingText(str);
	ft->setPosition(_vertices[idx].pos);

	addObject(ft);
	_text = ft;
	
	if (hover == false && !left) // click!
	{
		_confSpaceView->prepareMenu(group.object(idx));
	}
}
