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

#ifndef __vagabond__GuiDensity__
#define __vagabond__GuiDensity__

#include "Renderable.h"

class AtomMap;
class AtomGroup;

namespace MC
{
	struct mcMesh;
};

class GuiDensity : public Renderable
{
public:
	GuiDensity();

	virtual void render(SnowGL *gl);
	void populateFromMap(AtomMap *map);
	void recalculate();
	
	void setAtoms(AtomGroup *atoms)
	{
		_atoms = atoms;
	}

	virtual void extraUniforms();
private:
	void objectFromMesh(MC::mcMesh &mesh);
	AtomGroup *_atoms = nullptr;

	int _slice = 0;
};

#endif
