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

AbWatch::AbWatch(const SimplePolygon &other, const unsigned int &version_track, 
                 std::mutex *mut, const glm::vec3 &pos, const std::string &name)
: _versionTrack(version_track), _mut(mut), _pos(pos), _name(name)
{
	_vertices = other.vertices();
	_indices = other.indices();
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

	if (_currVersion >= _versionTrack)
	{
		return;
	}
	
	_currVersion = _versionTrack;
	_last = _pos;
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

void AbWatch::highlight()
{
	if (isSelectable())
	{
		setHighlighted(true);
	}

	FloatingText *text = new FloatingText(_name, 150, 10.);
	text->FloatingText::setPosition(_last);
	addTempObject(text);
}

void AbWatch::interacted(int idx, bool hover, bool left)
{
	if (hover)
	{
		highlight();
	}
}

void AbWatch::render(GLView *gl)
{
	glEnable(GL_DEPTH_TEST);

	_model = gl->getModel();
	Renderable::render(gl);

	glDisable(GL_DEPTH_TEST);
}
