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

#include <vagabond/gui/elements/Renderable.h>
#include <vagabond/core/Atom.h>

class AtomGroup;
class GuiAtom;

class GuiRepresentation : public Renderable
{
public:
	GuiRepresentation(GuiAtom *parent)
	{
		_parent = parent;
	}
	
	virtual ~GuiRepresentation() {}

	virtual void updateSinglePosition(Atom *a, glm::vec3 &p) = 0;
	virtual void updateMultiPositions(Atom *a, Atom::WithPos &wp) = 0;
	virtual void watchAtom(Atom *a) = 0;

	virtual void prepareAtomSpace(AtomGroup *ag) = 0;
protected:
	GuiAtom *parent()
	{
		return _parent;
	}
private:
	GuiAtom *_parent = nullptr;

};

#endif
