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

#ifndef __vagabond__GuiThickBond__
#define __vagabond__GuiThickBond__

#include "GuiRepresentation.h"

class GuiAtom;

class GuiThickBond : public GuiRepresentation
{
public:
	GuiThickBond(GuiAtom *parent);
	virtual ~GuiThickBond();

	virtual void watchAtom(Atom *a) {}
	virtual void watchAtomGroup(AtomGroup *ag);

	virtual void updateSinglePosition(Atom *a, glm::vec3 &p);
	virtual void updateMultiPositions(Atom *a, Atom::WithPos &wp);
	
	virtual void prepareAtomSpace(AtomGroup *ag);
	virtual void addVisuals(Atom *a);

	virtual void removeVisuals();
private:
	void addBond(Atom *left, Atom *right);

	size_t verticesPer()
	{
		return 4;
	}

	size_t indicesPer()
	{
		return 6;
	}
	
	struct AtomMarker
	{
		Atom *left;
		Atom *right;
		size_t idx;
		int opaque_request;
	};
	
	std::map<Atom *, std::vector<size_t> > _atom2Markers;
	std::vector<AtomMarker> _markers;
};

#endif
