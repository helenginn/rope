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

#include <vagabond/gui/elements/FloatingText.h>
#include "AbWatch.h"

// the label is a child of the AbWatch, which is drawn with depth testing on,
// and sits inside the antibody: draw it without depth so it isn't hidden.
class AbLabel : public FloatingText
{
public:
	using FloatingText::FloatingText;

	virtual void render(GLView *gl)
	{
		GLboolean was = glIsEnabled(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_TEST);
		FloatingText::render(gl);

		if (was)
		{
			glEnable(GL_DEPTH_TEST);
		}
	}
};

AbWatch::AbWatch(const SimplePolygon &other, const Source &source, 
                 std::mutex *mut, const std::string &name)
: _source(source), _mut(mut), _name(name)
{
	_vertices = other.vertices();
	_indices = other.indices();
	_baseColour = (_vertices.size() ? _vertices[0].color : glm::vec4(1.f));
	_hc = {0.5, 0.5, 0.9};
	_selectionResize = 1.3;

	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/lighting.fsh");
}

void AbWatch::doThings()
{
	 std::unique_lock<std::mutex> lock(*_mut, std::defer_lock);

	if (!lock.try_lock())
	{
		return;
	}

	glm::vec3 where;
	unsigned int version = _source(where);

	if (_started && version == _currVersion)
	{
		return;
	}

	_started = true;
	_currVersion = version;
	_last = where;
	if (_last.x == _last.x)
	{
		setPosition(_last);
		forceRender(true, false);
	}
}

void AbWatch::reindex()
{
	size_t offset = indexOffset();
	for (size_t i = 0; i < vertexCount(); i++)
	{
		/* in the case of multiple responders */
		_vertices[i].extra[0] = i + offset + 1.5;
	}
}

void AbWatch::setFixedStyle(bool fixed)
{
	// a highlighted copy would bring its old colours back when let go
	if (isSelected())
	{
		setHighlighted(false);
	}

	setSelectable(!fixed);

	if (fixed)
	{
		for (Vertex &v : _vertices)
		{
			v.color = _baseColour;
		}
	}
	else
	{
		setColour(0.2, 0.2, 0.6);
	}

	forceRender(true, false);
}

void AbWatch::highlightQuietly()
{
	if (isSelectable())
	{
		setHighlighted(true);
	}
}

void AbWatch::highlight()
{
	highlightQuietly();

	// already labelled: don't stack another label on every mouse move
	if (_temps.size())
	{
		return;
	}

	FloatingText *text = new AbLabel(_name, 150, 10.);
	text->FloatingText::setPosition(_last);
	addTempObject(text);
}

void AbWatch::interacted(int idx, bool hover, bool left)
{
	if (hover)
	{
		if (_hoverJob && !_hoverJob(_antibody, _copy))
		{
			return;
		}

		highlight();
	}
	else if (left && _clickJob)
	{
		_clickJob(this);
	}
}

void AbWatch::render(GLView *gl)
{
	glEnable(GL_DEPTH_TEST);

	_model = gl->getModel();
	Renderable::render(gl);

	glDisable(GL_DEPTH_TEST);
}
