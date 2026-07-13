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

#ifndef __vagabond__GuiAtomConnections__
#define __vagabond__GuiAtomConnections__

#include "GuiRepresentation.h"
#include <map>
#include <set>
#include <utility>
#include <vector>

class GuiAtom;

class GuiAtomConnections : public GuiRepresentation
{
public:
	typedef std::pair<Atom *, Atom *> AtomConnection;

	GuiAtomConnections(GuiAtom *parent);
	virtual ~GuiAtomConnections() {}

	void setConnections(const std::vector<AtomConnection> &connections);
	size_t connectionCount() const
	{
		return _markers.size();
	}

	virtual void watchAtom(Atom *a) {}
	virtual void watchAtomGroup(AtomGroup *ag) {}
	virtual void updateSinglePosition(Atom *a, glm::vec3 &p);
	virtual void updateMultiPositions(Atom *a, WithPos &wp);
	virtual void setHidden(Atom *a, const bool &hidden);

private:
	struct AtomMarker
	{
		Atom *left = nullptr;
		Atom *right = nullptr;
		size_t idx = 0;
	};

	void addConnection(Atom *left, Atom *right);
	void updateMarker(AtomMarker &marker);
	void addArrow(glm::vec3 start, glm::vec3 end, float alpha);
	void addSegment(glm::vec3 start, glm::vec3 end, const glm::vec4 &colour,
	                float half_width);
	void setArrow(size_t idx, glm::vec3 start, glm::vec3 end, float alpha);
	void setSegment(size_t idx, glm::vec3 start, glm::vec3 end,
	                const glm::vec4 &colour, float half_width);
	glm::vec3 positionFor(Atom *atom) const;

	std::map<Atom *, std::vector<size_t>> _atom2Markers;
	std::vector<AtomMarker> _markers;
	std::set<Atom *> _hidden;
};

#endif
