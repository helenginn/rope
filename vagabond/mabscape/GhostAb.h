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

#ifndef __vagabond__GhostAb__
#define __vagabond__GhostAb__

#include <vagabond/gui/elements/SimplePolygon.h>

/** a translucent black antibody which swells and shrinks, to draw the eye to
 *  where an antibody is known to belong. Not pickable. */
class GhostAb : public SimplePolygon
{
public:
	GhostAb();

	void show(const glm::vec3 &where);
	void hide();

	// Over a solid antigen the depth it has written is kept, so that the
	// ghost is hidden where it is behind the antigen
	void setOverSolid(bool over)
	{
		_overSolid = over;
	}

	virtual void render(GLView *gl);
private:
	std::vector<glm::vec3> _offsets; // from the centre, at full size
	glm::vec3 _centre{};
	bool _overSolid{false};
};

/** a line between two points */
class GhostLine : public SimplePolygon
{
public:
	GhostLine();

	void show(const glm::vec3 &from, const glm::vec3 &to);
	void hide();
};

#endif
