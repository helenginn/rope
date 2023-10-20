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

#ifndef __vagabond__GuiRepresentation__
#define __vagabond__GuiRepresentation__

#include <vagabond/gui/elements/IndexResponder.h>
#include <vagabond/core/AtomPosMap.h>
#include "ColourScheme.h"

#define POINTS_PER_BEZIER (10)

class AtomGroup;
class GuiAtom;

class GuiRepresentation : public IndexResponder
{
public:
	GuiRepresentation(GuiAtom *parent)
	{
		_parent = parent;
		setName("Gui representation");
	}

	virtual size_t requestedIndices()
	{
		return 0;
	}
	
	virtual ~GuiRepresentation()
	{
		delete _scheme;
	}

	virtual void updateSinglePosition(Atom *a, glm::vec3 &p) = 0;
	virtual void finishUpdate() {};
	virtual void updateMultiPositions(Atom *a, WithPos &wp) = 0;
	virtual void watchAtom(Atom *a) = 0;
	virtual void watchAtomGroup(AtomGroup *ag);

	virtual void prepareAtomSpace(AtomGroup *ag) {}

	virtual void removeVisuals()
	{
		setAlpha(0.f);
	}

	virtual void addVisuals(Atom *a) {};
protected:
	float control_points(glm::vec3 *a, glm::vec3 b, 
	                     glm::vec3 c, glm::vec3 *d);
	std::vector<Snow::Vertex> bezierFrom(std::vector<Snow::Vertex> vs,
	                                     int idx);
	glm::vec3 bezier(glm::vec3 p1, glm::vec3 p2, 
	                 glm::vec3 p3, glm::vec3 p4, double t);

	std::vector<Snow::Vertex> wireFrameToBezier(std::vector<Snow::Vertex> &vs,
	                                            int buffer = 0);
	GuiAtom *parent()
	{
		return _parent;
	}

	void addCylinderIndices(std::vector<Snow::Vertex> &vertices,
	                        std::vector<GLuint> &indices, size_t num);
	Snow::Vertex new_vertex(glm::vec3 p);
	std::vector<Snow::Vertex> makeBezier(glm::vec3 p1, glm::vec3 p2,
	                                     glm::vec3 p3, glm::vec3 p4,
	                                     bool overwrite = false);

	ColourScheme *_scheme = new ColourScheme(Heat);
private:
	GuiAtom *_parent = nullptr;

};

#endif
